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
// #include "BBBiolib.h"
#include "trace.h"
#include "typedefs.h"
#include "sys_dat.h"

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
unsigned short 	semval;
union semun {
	int val;              						// used for SETVAL only
	struct semid_ds *buf; 						// for IPC_STAT and IPC_SET
	ushort *array;        						// used for GETALL and SETALL
};
union 			semun dummy;
SEMBUF sb = {0, -1, 0};  /* set to allocate resource */

/********** support functions *******************************************************************/

int pin_high(int h, int p){

	static char 	*prefix="config-pin P";
	char 			header[4];
	char 			pin[5];
	char 			cmd[30];


	sprintf(header, "%d", h);
	sprintf(pin, "%d", p);
	strcpy(cmd,prefix);
	strcat(cmd,header);
	strcat(cmd,".");
	strcat(cmd,pin);
	strcat(cmd," high");
	system(cmd);

	return 0;
}

int pin_low(int h, int p){

	static char 	*prefix="config-pin P";
	char 			header[4];
	char 			pin[5];
	char 			cmd[30];

	sprintf(header, "%d", h);
	sprintf(pin, "%d", p);
	strcpy(cmd,prefix);
	strcat(cmd,header);
	strcat(cmd,".");
	strcat(cmd,pin);
	strcat(cmd," low");
	system(cmd);

	return 0;
}

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
	// uint8_t 			ccb;
	// int 				i;
	static _GPIO 		gpio[_NUMBER_OF_CHANNELS] = {_CHAN0,_CHAN1,_CHAN2,_CHAN3,_CHAN4,_CHAN5,_CHAN6,_CHAN7,_CHAN8,_CHAN9,_CHAN10,_CHAN11,_CHAN12,_CHAN13,_CHAN14,_CHAN15};

// char debug_buff[128];
	
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	key =  tm->tm_hour * 60 + tm->tm_min;		// generate key
	// logit(" ");
	logit("starting channel update *******************************************");
	/* set channel state based on channel mode */
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		// sprintf(debug_buff,"updating channel %d",channel);
		// logit(debug_buff);
		switch (ipc_ptr->sys_data.c_data[channel].mode) {
		case 0:	// manual
			// logit("manual control");
			state = ipc_ptr->sys_data.c_data[channel].state;
			break;
		case 1:	// time
			// logit("time control");
			state =  test_sch_time(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]));
			// ----------------------------------------------------------------------------------------------------------
			// // char 			buff[128];
			// // sprintf(buff, "current time generated key %i,  %i returned from test_sch_time\ndump schedule\n",key, state);
			// // logit(buff);
			// for(i=0;i<ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rcnt;i++){
			// 	sprintf(buff, "  schedule record %i, key= %i, state=%i, channel=%i",
			// 		    i, ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rec[i].key, ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rec[i].state, channel);
			// 	logit(buff);
			// }
			// -------------------------------------------------------------------------------------------------------------

						// ipc_ptr->sys_data.c_data[channel].state = state;
			break;
		case 2:	// time & sensor
			// logit("sensor & time control");
			state =  test_sch_sensor(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]), ipc_ptr->s_dat[ipc_ptr->sys_data.c_data[channel].sensor_id].temp);

			// ipc_ptr->sys_data.c_data[channel].state = state;
			break;
		case 3:	// cycle
			logit("*** error mode set to <3>");
			break;
		default: // error
			logit("*** error mode set to a bad value");
		}
		ipc_ptr->sys_data.c_data[channel].state = state;
	}
	// logit("end channel update *******************************************");
	logit("starting pin update *******************************************");
	/* update gpio pins for all channels */
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		// sprintf(debug_buff,"updating pin %d",channel);
		// logit(debug_buff);
		if (ipc_ptr->sys_data.c_data[channel].state) {
			pin_high(gpio[channel].header, gpio[channel].pin);
			// logit("pin set high");
		}
		else {
			pin_low(gpio[channel].header, gpio[channel].pin);
			// logit("pin set low");
		}
	}

	ipc_sem_free(semid, &sb);	// free lock on shared memory
	// send_ccb(ccb);         		// send control byte to the DIOB
	logit("end pin update *******************************************");
	return;
}

int main(void) {

	/* Our process ID and Session ID */
	pid_t 		pid, sid;
	int 		toggle;
	int 		h_min;
	_tm 		t;
	int 		ipc;
	FILE 		*pidf;
	static _GPIO 		heart = {.header=8, .pin=27};

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


	logit("\n*****************\ndaemon started");
	logit("starting initializations");

	/* check for ipc file and ipc backup file */	
    // if( access(_IPC_FILE_BACKUP_NAME, F_OK ) != -1 ){
    //     bkup = 1;
    //     fprintf(stderr, "%s\n"," ipc backup found" );
    // }
    // else{ 
    //     bkup = 0;
    //     fprintf(stderr, "%s\n"," ipc backup not found" );
    // }
    if( access(_IPC_FILE_NAME, F_OK ) != -1 ){
        ipc = 1;
        logit(" ipc file found" );

    }
    else
        ipc = 0;
    

	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (_IPC_DAT *)data;					// overlay ipc data structure on shared memory

	if(ipc==0){
		logit(" ipc file not found" );
		logit(" new ipc file created and initialized" );
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

	/* The Big Loop */
	logit("initialization complete");
	logit("starting main loop");
	while (1) {

		get_tm(&t);

		if (ipc_ptr->force_update == 1) {
			ipc_sem_lock(semid, &sb);                   // wait for a lock on shared memory
			ipc_ptr->force_update = 0;
			ipc_sem_free(semid, &sb);                   // free lock on shared memory
			logit("*********************");
			logit("update forced");
			update_relays(&t, ipc_ptr);
			continue;
		}
		else {
			if (h_min != t.tm_min) {	// see if we are on a new minute
				h_min = t.tm_min;
				logit("*********************");
				logit("update triggered by time");
				update_relays(&t, ipc_ptr);
				continue;
			}
		}
		if (toggle) {					// cycle cape leds
			toggle = 0;
			pin_low(heart.header,heart.pin);

		}
		else {
			toggle = 1;
			pin_high(heart.header,heart.pin);

		}
		// usleep(300000);
	}

	exit(EXIT_SUCCESS);
}

