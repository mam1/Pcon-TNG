/********************************************************************/
/*	Dcon.c - daemon updates the channel relays once a minute unless */
/*	the user requests an immediate update							*/
/********************************************************************/

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "shared.h"
#include "bitlit.h"
#include "PCF8563.h"
#include "schedule.h"
#include "BBBiolib.h"
#include "trace.h"

/*********************** globals **************************/
#ifdef _TRACE
char			trace_buf[128];
#endif
int             	trace_flag;                   	//trace file is active

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *c_mode[4] = {"manual", "  time", "   t&s", " cycle"};

/********** globals *******************************************************************/
IPC_DAT        	ipc_dat, *ipc_ptr;                    		// ipc data
char           	ipc_file[] = {_IPC_FILE};   				// name of ipc file
void           	*data;                      				// pointer to ipc data
int            	fd;                        				 	// file descriptor for ipc data file
int         	rtc;										// file descriptor for PCF8563 RTC
_tm         	tm;											// time date structure
key_t 			skey = _SEM_KEY;
int 			semid;
unsigned short 	semval;
// struct sembuf 	wait, signal;
union semun {
	int val;              /* used for SETVAL only */
	struct semid_ds *buf; /* for IPC_STAT and IPC_SET */
	ushort *array;        /* used for GETALL and SETALL */
};
union 			semun dummy;
SEMBUF sb = {0, -1, 0};  /* set to allocate resource */

/********** support functions *******************************************************************/
void dispdat(void) {
	int         i;
	printf("  channel  state   mode   \n\r");
	printf("  ---------------------------------\n\r");
	for (i = 0; i < _NUMBER_OF_CHANNELS; i++) {
		printf("   <%2i> - ", i);
		printf(" %s     %s", onoff[ipc_ptr->c_dat[i].c_state], c_mode[ipc_ptr->c_dat[i].c_mode]);
		if ((ipc_ptr->c_dat[i].c_mode) == 3)
			printf(" (%i:%i)", ipc_ptr->c_dat[i].on_sec, ipc_ptr->c_dat[i].off_sec);
		printf("\r\n");
	}
	return;
}

void update_relays(_tm *tm, IPC_DAT *ipc_ptr) {

	int 				key;
	uint32_t			*s_ptr;		// *r_ptr;
	int 				state;
	int 				channel;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	// disp_sch((uint32_t *)ipc_ptr->sch);
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		switch (ipc_ptr->c_dat[channel].c_mode) {
		case 0:	// manual
			state = ipc_ptr->c_dat[channel].c_state;
			break;
		case 1:	// time
			key =  make_key(tm->tm_hour, tm->tm_min);							// generate key
			s_ptr = get_schedule(((uint32_t *)ipc_ptr->sch), tm->tm_wday, channel); 	// get a pointer to schedule for (day,channel)
			// s_ptr++;
			// printf("got s_ptr\n");
			// r_ptr = find_schedule_record(s_ptr,key);  							// search schedule for record with key match, return pointer to record or NULL
			// printf("got r_ptr <%x>\n",(uint32_t)r_ptr);
			state =  test_sch(s_ptr, key);
			// printf("got new state <%i>\n", state);
			ipc_ptr->c_dat[channel].c_state = state;
			break;
		case 2:	// time & sensor
			printf("*** error mode set to <2>\n");
			break;
		case 3:	// cycle
			printf("*** error mode set to <3>\n");
			break;
		default: // error
			printf("*** error mode set to <%i>\n", ipc_ptr->c_dat[channel].c_mode);
		}
	#ifdef _TRACE
		sprintf(trace_buf, "    Dcon:update_relays:  relay %i set to %i\n", channel, state);
		strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
	#endif	
	}
	ipc_sem_free(semid, &sb);					// free lock on shared memory
	return;
}

/* initialise gpio pins for iolib */
int init_gpio(void){
	char 				command[60];

	strcpy( command, "echo  66 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	strcpy( command, "echo  67 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  69 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  68 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  45 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  44 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  26 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  47 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  46 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  27 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  65 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  30 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  60 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  31 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  48 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  49 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo 117 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo 115 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo 112 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);
	strcpy( command, "echo  20 > /sys/class/gpio/export");
	printf("system command %s returned %i\n", command,system(command));
	system(command);

	return 0;
}

int main(void) {

	int 	h_min;
	int 	toggle;

	printf("\n  **** daemon active 0.8 ****\n\n");

	/********** initializations *******************************************************************/

	/* setup trace */
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active,");
		if (trace_on(_TRACE_FILE_NAME, &trace_flag)) {
			printf("trace_on returned error\n");
			trace_flag = false;
		}
	}
	if (trace_flag == false)
		printf(" program trace disabled\n");

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus

	/* setup shared memory */
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (IPC_DAT *)data;					// overlay ipc data structure on shared memory
	ipc_ptr->force_update = 1;
	disp_sch((uint32_t *)ipc_ptr->sch);
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* initialise gpio access */
	init_gpio();
	iolib_init();

	/* setup gpio access to LEDs on the WaveShare Cape*/						
	printf(" iolib initialized\n");
	printf("\n mapping WaveShare Misc Cape leds to gpio pins\n");
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
	printf("\n mapping PhotoMos relays to gpio pins\n");
	iolib_setdir(9, _R1_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R2_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R3_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R4_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R5_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R6_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R7_CAPE, BBBIO_DIR_OUT);
	iolib_setdir(9, _R8_CAPE, BBBIO_DIR_OUT);


	/* setup gpio access to serial header on the DIOB */
	printf("\n mapping DIOB serial header to gpio pins\n");
	iolib_setdir(8, _DIOB_DIN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_DATA_RLY, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_SCLK_IN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_SCLK_RLY, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_LOAD_IN, BBBIO_DIR_OUT);


	/********** main loop *******************************************************************/

	printf("starting main loop\n");
	while (1) {

		if (ipc_ptr->force_update == 1) {
			ipc_ptr->force_update = 0;
			printf("\n*** update forced\n");
			update_relays(&tm, ipc_ptr);
			dispdat();
		}
		else {
			get_tm(rtc, &tm);
			if (h_min != tm.tm_min) {					// see if we are on a new minute
				h_min = tm.tm_min;
				printf("\n*** update triggered by time:");
				printf("  %02i:%02i:%02i  %02i/%02i/%02i  dow %i\n",
				       tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mon, tm.tm_mday, tm.tm_year, tm.tm_wday);
				update_relays(&tm, ipc_ptr);
				dispdat();
			}
		}

		/* cycle leds */
		if (toggle) {
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
		sleep(1);

	}

	return 0;
}