/********************************************************************/
/*	Dcon.c - daemon updates the channel relays once a minute unless */
/*	the user requests an immediate update							*/
/********************************************************************/

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

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char *onoff[2] = {"off"," on"};
char *con_mode[3] = {"manual","  time","time & sensor"};
char *sch_mode[2] = {"day","week"};
char *c_mode[4] = {"manual","  time","   t&s"," cycle"};	

/********** globals *******************************************************************/

IPC_DAT        	ipc_dat, *ipc_ptr;                    		// ipc data
char           	ipc_file[] = {_IPC_FILE};   				// name of ipc file
void           	*data;                      				// pointer to ipc data
int            	fd;                        				 	// file descriptor for ipc data file
int         	rtc;										// file descriptor for PCF8563 RTC
_tm         	tm;											// time date structure

/********** support functions *******************************************************************/
void dispdat(void) {
	int 		channel;

	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		if (channel < 10)
			printf("  %i  - %i\n", channel, ipc_ptr->c_dat[channel].c_state );
		else
			printf("  %i - %i\n", channel, ipc_ptr->c_dat[channel].c_state );
	}
	return;
}

void update_relays(_tm *tm, IPC_DAT *sm) {

	int 				key;
	uint32_t			*srec_ptr;
	int 				state;
	int 				channel;

	key =  make_key(tm->tm_hour, tm->tm_min);						// generate key
	printf("key = %i\n",key );
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		srec_ptr = get_schedule(sm->sch,tm->tm_wday,channel);		// get a pointer to record matching key
		state =  get_s(*srec_ptr);									// extract state from schedule record
		printf("chanel = %i, state = %i\n", channel, state);
		// get a memory lock
	 	sm->c_dat[channel].c_state = state;
//		memcpy(data, ipc_ptr, sizeof(ipc_dat));  	// move local structure to shared memory


	}
	printf("  %02i:%02i:%02i  %02i/%02i/%02i  dow %i\n",
	       tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon, tm->tm_mday, tm->tm_year, tm->tm_wday);
	return;
}


int main(void) {

	int 	h_min;
	int 	toggle;

	printf("\n  **** daemon active 0.4 ****\n\n");

	/********** initializations *******************************************************************/

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus
	/* setup shared memory */
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = data;
	// memcpy(ipc_ptr, data, sizeof(ipc_dat));  	// move shared memory data to local structure
	printf("\n  force_update <%i>\n\n", ipc_ptr->force_update);
	
	/* setup semaphores */
	ipc_init_sem();

	/* setup gpio access */
	iolib_init();
  	iolib_setdir(8, _LED_1, BBBIO_DIR_OUT);
  	iolib_setdir(8, _LED_2, BBBIO_DIR_OUT);
  	iolib_setdir(8, _LED_3, BBBIO_DIR_OUT);
  	iolib_setdir(8, _LED_4, BBBIO_DIR_OUT);
  	pin_low(8,  _LED_1);
  	pin_low(8,  _LED_2);
  	pin_low(8,  _LED_3);
  	pin_low(8,  _LED_4);

	/********** main loop *******************************************************************/
	printf("starting main loop\n");
	while (1) {
//		memcpy(ipc_ptr, data, sizeof(ipc_dat));  		// move shared memory data to local structure
		if (ipc_ptr->force_update) {
			dispdat();
			ipc_ptr->force_update = 0;
			printf("  update forced\n");
			// wait for a lock on shared memory
//			memcpy(data, ipc_ptr, sizeof(ipc_dat));  	// move local structure to shared memory
			// unlock shared memory
//			update_relays(&tm, ipc_ptr);
			dispdat();
		}
		else {
			get_tm(rtc, &tm);
			if (h_min != tm.tm_min) {					// see if we are on a new minute
				h_min = tm.tm_min;
				// update_relays(&tm, ipc_ptr);
				dispdat();
			}
		}

		/* cycle leds */
		if(toggle){
			toggle = 0;
		  	pin_low(8,  _LED_1);
		  	pin_high(8,  _LED_2);
		  	pin_low(8,  _LED_3);
		  	pin_high(8,  _LED_4);
		}
		else{
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