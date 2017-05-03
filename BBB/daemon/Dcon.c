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
#include "BBBiolib.h"
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

/* send control byte to dio board */
void send_ccb(uint8_t byte)
{
	int         i;
	pin_low(8, _DIOB_DIN);
	for (i = 7; i >= 0; i--)   			//serialize and reverse bits
	{
		if ((1 << i) & byte)
			pin_high(8, _DIOB_DIN); //send bit high
		else
			pin_low(8, _DIOB_DIN);  //send bit low
		/* send clock pluse */
		pin_high(8, _DIOB_SCLK_IN);
		pin_low(8, _DIOB_SCLK_IN);
	}
	pin_high(8, _DIOB_LAT_RLY);      //set the LAT_RlY to high this will cause the HC595 to read the value from the shift register */
	pin_low(8, _DIOB_LAT_RLY);       //done - ready for next write */
	return;
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
	uint8_t 			ccb;
	int 				gpio_index;
	static int 			pin[_NUMBER_OF_GPIOS] = {_PINS};
	static int 			header[_NUMBER_OF_GPIOS] = {_HEADERS};
	int 				i;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	key =  tm->tm_hour * 60 + tm->tm_min;		// generate key

	/* set channel state based on channel mode */
	// logit("updating relays");
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		switch (ipc_ptr->sys_data.c_data[channel].mode) {
		case 0:	// manual
			logit("manual control");
			state = ipc_ptr->sys_data.c_data[channel].state;
			break;
		case 1:	// time
			logit("time control");
			state =  test_sch_time(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]));
// ----------------------------------------------------------------------------------------------------------
char 			buff[128];
sprintf(buff, "current time generated key %i,  %i returned from test_sch_time\ndump schedule\n",key, state);
logit(buff);
for(i=0;i<ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rcnt;i++){
	sprintf(buff, "  schedule record %i, key= %i, state=%i, channel=%i",
		    i, ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rec[i].key, ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel].rec[i].state, channel);
	logit(buff);
}
// -------------------------------------------------------------------------------------------------------------

			// ipc_ptr->sys_data.c_data[channel].state = state;
			break;
		case 2:	// time & sensor
			logit("sensor & time control");
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

	/* update on board relays channels 0-7 */
	for (channel = 0; channel < 8; channel++) {
		gpio_index = channel + 8;
		if (ipc_ptr->sys_data.c_data[channel].state) {
			pin_high(header[gpio_index], pin[gpio_index]);
		}
		else {
			pin_low(header[gpio_index], pin[gpio_index]);
		}
	}

	/* update DIOB relays channels 8-15 */
	for (channel = 8; channel < 16; channel++) {
		if (ipc_ptr->sys_data.c_data[channel].state)
			ccb |= (1 << ((channel - 8)));
		else
			ccb &= ~(1 << ((channel - 8)));
	}
	ipc_sem_free(semid, &sb);	// free lock on shared memory
	send_ccb(ccb);         		// send control byte to the DIOB
	
	return;
}

/* initialise gpio pins for iolib */
int init_gpio(void) {
	char 				command[120];
	int 				i;
	int 				gpios[_NUMBER_OF_GPIOS] = {_GPIOS};

	for (i = 0; i < _NUMBER_OF_GPIOS; i++) {
		sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpios[i], gpios[i] );
		logit(command);
		printf("system command %s returned %i\n", command, system(command));
	}

	return 0;
}



int main(void) {

	/* Our process ID and Session ID */
	pid_t 		pid, sid;
	int 		toggle;
	int 		h_min;
	_tm 		t;
	int 		ipc;
	FILE 		*pidf;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		/* create pid file */
		fopen($PID_FILE_NAME, "w");
		if (pidf != NULL){
			
			fprintf(pidf,"%i", pid);
			fclose(pidf);
		}
		else{
			printf(" can't write pid file <%s>\n", PID_FILE_NAME);
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open log file */

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

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus
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

	/* initialise gpio access */
	init_gpio();
	iolib_init();

	/* setup gpio access to LEDs on the WaveShare Cape*/
	logit("iolib initialized");
	logit("mapping WaveShare Misc Cape leds to gpio pins");
	iolib_setdir(8, _LED_1, BBBIO_DIR_OUT);
	iolib_setdir(8, _LED_2, BBBIO_DIR_OUT);
	iolib_setdir(8, _LED_3, BBBIO_DIR_OUT);
	iolib_setdir(8, _LED_4, BBBIO_DIR_OUT);

	/* turn off the LEDs */
	pin_low(8, _LED_1);
	pin_low(8, _LED_2);
	pin_low(8, _LED_3);
	pin_low(8, _LED_4);

	/* setup gpio access to PhotoMos relays */
	logit("mapping PhotoMos relays to gpio pins");
	iolib_setdir(8, _R1_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(8, _R2_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(8, _R3_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(8, _R4_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R5_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R6_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R7_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R8_CAPE, BBBIO_DIR_OUT);

	/* turn all relays off */
	pin_low(8, _R1_CAPE);
	pin_low(8, _R2_CAPE);
	pin_low(8, _R3_CAPE);
	pin_low(8, _R4_CAPE);
	pin_low(9, _R5_CAPE);
	pin_low(9, _R6_CAPE);
	pin_low(9, _R7_CAPE);
	pin_low(9, _R8_CAPE);

	/* setup gpio access to serial header on the DIOB */
	logit("mapping DIOB serial header to gpio pins");
	iolib_setdir(8, _DIOB_DIN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_SCLK_IN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_LAT_RLY, BBBIO_DIR_OUT);
	pin_low(8, _DIOB_SCLK_IN);
	pin_low(8, _DIOB_DIN);

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
		if (toggle) {					// cycle cape leds
			toggle = 0;
			pin_low(8,  _LED_1);
			pin_high(8,  _LED_2);
			pin_low(8,  _LED_3);
			pin_high(8,  _LED_4);
		}
		else {
			toggle = 1;
			pin_high(8,  _LED_1);
			pin_low(8,  _LED_2);
			pin_high(8,  _LED_3);
			pin_low(8,  _LED_4);
		}
		usleep(300000);
	}

	exit(EXIT_SUCCESS);
}

