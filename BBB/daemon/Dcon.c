/********************************************************************/
/*	Dcon.c - daemon updates the channel relays once a minute unless */
/*	the user requests an immediate update							*/
/********************************************************************/

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include "Pcon.h"
#include "Dcon.h"
#include "ipc.h"
#include "bitlit.h"
#include "PCF8563.h"
#include "sch.h"
#include "trace.h"
#include "typedefs.h"
#include "sys_dat.h"
// #include "bbb.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *mode[4] = {"manual", "  time", "   t&s", " cycle"};

/********** globals *******************************************************************/
_IPC_DAT       	ipc_dat, *ipc_ptr;              // ipc data
char           	ipc_file[] = {_IPC_FILE_NAME};  // name of ipc file
void           	*data;                      	// pointer to ipc data
int            	fd;                        		// file descriptor for ipc data file
int         	rtc;							// file descriptor for PCF8563 RTC
_tm         	tm;								// time date structure
key_t 			skey = _SEM_KEY;
int 			semid;

/* global memory mapped io variables */
unsigned short 	semval;
union semun {
	int val;              						// used for SETVAL only
	struct semid_ds *buf; 						// for IPC_STAT and IPC_SET
	ushort *array;        						// used for GETALL and SETALL
};
union 			semun dummy;
SEMBUF sb = {0, -1, 0};  						// set to allocate resource 

/* gpio assignments */
static _GPIO 			chan[16] = {_CHAN0,_CHAN1,_CHAN2,_CHAN3,_CHAN4,_CHAN5,_CHAN6,_CHAN7,_CHAN8,_CHAN9,_CHAN10,_CHAN11,_CHAN12,_CHAN13,_CHAN14,_CHAN15};
static _GPIO 			heart[4] = {_HB0, _HB1, _HB2, _HB3};
char 				command[120];

/********** support functions *******************************************************************/

/* write an entry to the daemon log file */
void logit(char *mess){
	FILE 		*dlog;
	_tm 		tm;

	get_tm(&tm); 		// load my time date structure from system clock

	/* Open log file */
	dlog = fopen(_DAEMON_LOG, "a");
	if (dlog == NULL) {
		exit(EXIT_FAILURE);
	}

	fprintf(dlog,"%02i:%02i:%02i  %02i/%02i/%02i  %s\n",tm.tm_hour,tm.tm_min,tm.tm_sec,tm.tm_mon,tm.tm_mday,tm.tm_year, mess);
	fclose(dlog);
	return;
}

/* update all relays */
void update_relays(_tm *tm, _IPC_DAT *ipc_ptr) {
	int 				key;
	int 				state;
	int 				channel;

	logit("trying for a lock on shared memory");
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	key =  tm->tm_hour * 60 + tm->tm_min;		// generate key
	logit("starting channel update");
	/* set channel state based on channel mode */
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		switch (ipc_ptr->sys_data.c_data[channel].mode) {
		case 0:	// manual
			state = ipc_ptr->sys_data.c_data[channel].state;
			break;
		case 1:	// time
			state =  test_sch_time(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]));
			// FILE *saved = stdout;
			// stdout = fopen(_DAEMON_LOG, "a");
			// printf("    channel %i conreoled by time, test_sch_time returned <%i>\n", channel, state);
			// fclose(stdout);
			// stdout = saved;
			break;
		case 2:	// time & sensor
			state =  test_sch_sensor(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]), ipc_ptr->s_dat[ipc_ptr->sys_data.c_data[channel].sensor_id].temp);
			break;
		case 3:	// cycle
			logit("*** error channel mode set to 3 - cycle mode not supported");
			break;
		default: // error
			logit("*** error channel mode set to a bad value");
		}
		ipc_ptr->sys_data.c_data[channel].state = state;
	}
	
	/* update gpio pins for all channels */
	logit("starting pin update");
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		if (ipc_ptr->sys_data.c_data[channel].state) {
			sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", chan[channel].gpio);
			// logit(command);
			system(command);
		}
		else {
			sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", chan[channel].gpio);
			// logit(command);
			system(command);
		}
	}
	ipc_sem_free(semid, &sb);	// free lock on shared memory
	logit("end pin update");
	return;
}

/* export gpio pin */
int init_gpio(int gpio)
{
	char 				command[120];
	sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpio, gpio);
	printf("system command %s returned %i\n", command, system(command));
	sprintf(command, "echo out > /sys/class/gpio/gpio%i/direction", gpio);
	printf("system command %s returned %i\n", command, system(command));
	sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", gpio);
	printf("system command %s returned %i\n", command, system(command));
	return 0;
}

int main(void) {
	
	pid_t 		pid, sid;		// process ID and Session ID
	int 		toggle;
	int 		h_min;
	_tm 		t;
	int 		ipc;
	FILE 		*pidf;
	char 		command[120];
	int 		i;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		/* create pid file */
		pidf = fopen(_PID_FILE_NAME, "w");
		if (pidf != NULL){
			
			fprintf(pidf,"%i", pid);
			fclose(pidf);
		}
		else{
			printf(" can't write pid file <%s>\n", _PID_FILE_NAME);
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Daemon-specific initializations */
	sprintf(command, "\n ************************************\n daemon %i.%i.%i started\n",_MAJOR_VERSION_Dcon, _MINOR_VERSION_Dcon, _MINOR_REVISION_Dcon);
	logit(command);
	logit("starting initializations"); 

	/* load cape that disables HDMI and gives me back the gpios */
	// sprintf(command, "echo 'cape-universalh' > /sys/devices/platform/bone_capemgr/slots");
	// logit(command);
	// system(command);

	/* check for ipc file */
	if (access(ipc_file, F_OK) == 0) {
		ipc = 1;
		logit("ipc file found");
	}
	else {
		ipc = 0;
		logit("* ipc file not found");
	}
 
	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (_IPC_DAT *)data;					// overlay ipc data structure on shared memory
    ipc_sem_free(semid, &sb);                   // free lock on shared memory


	if(ipc==0){
		logit("* new ipc file created and initialized" );
		ipc_sem_lock(semid, &sb);                   // wait for a lock on shared memory
        ipc_ptr->sys_data.config.major_version = _MAJOR_VERSION_system;
        ipc_ptr->sys_data.config.minor_version = _MINOR_VERSION_system;
        ipc_ptr->sys_data.config.minor_revision = _MINOR_REVISION_system;
        ipc_ptr->sys_data.config.channels = _NUMBER_OF_CHANNELS;
        ipc_ptr->sys_data.config.sensors = _NUMBER_OF_SENSORS;
        ipc_ptr->sys_data.config.commands = _CMD_TOKENS;
        ipc_ptr->sys_data.config.states = _CMD_STATES;
        ipc_sem_free(semid, &sb);                   // free lock on shared memory
	}

	/* intialize gpio pins */
	for (i = 0; i < _NUMBER_OF_CHANNELS; i++) {
	// printf(" P%i.%i", chan[i].header, chan[i].pin);
	init_gpio(chan[i].gpio);
	}

	for (i = 0; i < 4; i++) {
	// printf(" P%i.%i", heart[i].header, heart[i].pin);
	init_gpio(heart[i].gpio);
	}

	/* The Big Loop */
	logit("initialization complete");
	logit("starting main loop");
	while (1) {
		get_tm(&t);
		if (ipc_ptr->force_update == 1) {
			ipc_sem_lock(semid, &sb);                   // wait for a lock on shared memory
			ipc_ptr->force_update = 0;
			ipc_sem_free(semid, &sb);                   // free lock on shared memory
			logit("update forced");
			update_relays(&t, ipc_ptr);
			continue;
		}
		else {
			if (h_min != t.tm_min) {	// see if we are on a new minute
				h_min = t.tm_min;
				logit("update triggered by time");
				update_relays(&t, ipc_ptr);
				continue;
			}
		}
		if (toggle) {					// cycle heart beat led
			toggle = 0;
			sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", heart[0].gpio);
			system(command);
			sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", heart[1].gpio);
			system(command);
			sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", heart[2].gpio);
			system(command);
			sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", heart[3].gpio);
			system(command);			
		}
		else {
			toggle = 1;
			sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", heart[0].gpio);
			system(command);
			sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", heart[1].gpio);
			system(command);
			sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", heart[2].gpio);
			system(command);
			sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", heart[3].gpio);
			system(command);;
		}
		// usleep(30000);
	}
	exit(EXIT_SUCCESS);
}

