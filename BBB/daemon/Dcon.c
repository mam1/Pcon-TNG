/********************************************************************/
/*	Dcon.c - daemon updates the channel relays once a minute unless */
/*	the user requests an immediate update							*/
/********************************************************************/

/* Dcon version info */
#define _MAJOR_VERSION		2
#define _MINOR_VERSION 		0
#define _MINOR_REVISION		0

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
#include "Pcon.h"
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
void logit(_tm *t, char *mess){
	FILE 		*dlog;
	_tm 		tm;

	if(t != NULL)
		tm = *t;
	else
		get_tm(rtc, &tm); 		// read the real time clock

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

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	key =  tm->tm_hour * 60 + tm->tm_min;		// generate key

	/* set channel state based on channel mode */
	// logit(NULL, "updating relays");
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		switch (ipc_ptr->sys_data.c_data[channel].mode) {
		case 0:	// manual
			state = ipc_ptr->sys_data.c_data[channel].state;
			break;
		case 1:	// time
			state =  test_sch_time(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]));
			// ipc_ptr->sys_data.c_data[channel].state = state;
			break;
		case 2:	// time & sensor
			state =  test_sch_sensor(key, &(ipc_ptr->sys_data.sys_sch.sch[tm->tm_wday][channel]), ipc_ptr->s_dat[ipc_ptr->sys_data.c_data[channel].sensor_id].temp);

			// ipc_ptr->sys_data.c_data[channel].state = state;
			break;
		case 3:	// cycle
			logit(NULL, "*** error mode set to <3>");
			break;
		default: // error
			logit(NULL, "*** error mode set to a bad value");
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
		logit(NULL, command);
		printf("system command %s returned %i\n", command, system(command));
	}

	return 0;
}



int main(void) {

	/* Our process ID and Session ID */
	pid_t 		pid, sid;
	FILE 		*sys_file;
	int 		toggle;
	_SYS_DAT 	sdat;
	int 		h_min;
	_tm 		t;


	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
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
	logit(NULL, "daemon started");

	get_tm(rtc, &t); 					// get the current time and date
	logit(NULL, "starting initializations");

	/* setup shared memory */
	ipc_sem_init();								// setup semaphores
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (_IPC_DAT *)data;					// overlay ipc data structure on shared memory
	logit(NULL, "shared memory initialized");

	/* load data from system data file */
	sys_file = sys_open(_SYSTEM_FILE_NAME, &sdat); // create system file if it does not exist
	sys_load(sys_file, &sdat);
	ipc_ptr->force_update = 1;
	ipc_sem_free(semid, &sb);					// free lock on shared memory
	fclose(sys_file);
	logit(NULL, "system data loaded into shared memory");

	/* initialise gpio access */
	init_gpio();
	iolib_init();

	/* setup gpio access to LEDs on the WaveShare Cape*/
	logit(NULL, "iolib initialized");
	logit(NULL, "mapping WaveShare Misc Cape leds to gpio pins");
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
	logit(NULL, "mapping PhotoMos relays to gpio pins");
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
	logit(NULL, "mapping DIOB serial header to gpio pins");
	iolib_setdir(8, _DIOB_DIN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_SCLK_IN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_LAT_RLY, BBBIO_DIR_OUT);
	pin_low(8, _DIOB_SCLK_IN);
	pin_low(8, _DIOB_DIN);

	/* The Big Loop */
	logit(NULL, "initialization complete");
	logit(NULL, "starting main loop");
	while (1) {
		get_tm(rtc, &tm);				// read the time from the real time clock
		if (ipc_ptr->force_update == 1) {
			ipc_ptr->force_update = 0;
			logit(&tm, "update forced");
			update_relays(&tm, ipc_ptr);
			continue;
		}
		else {
			if (h_min != tm.tm_min) {	// see if we are on a new minute
				h_min = tm.tm_min;
				logit(&tm, "update triggered by time");
				update_relays(&tm, ipc_ptr);
				continue;
			}
		}
		if (toggle) {					// cycle leds
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

