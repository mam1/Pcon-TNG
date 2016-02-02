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
#include "gpio.h"
#include "led.h"
#include "schedule.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char *onoff[2] = {"off"," on"};
char *con_mode[3] = {"manual","  time","time & sensor"};
char *sch_mode[2] = {"day","week"};
char *c_mode[4] = {"manual","  time","   t&s"," cycle"};	

/********** globals *******************************************************************/

IPC_DAT        	ipc_dat;                    				// ipc data
char           	ipc_file[] = {_IPC_FILE};   				// name of ipc file
void           	*data;                      				// pointer to ipc data
int            	fd;                        				 	// file descriptor for ipc data file
int         	rtc;										// file descriptor for PCF8563 RTC
_tm         	tm;											// time date structure
int             led_fd[4];                               	// file descriptor array for leds
int             led_state[4];				             	// led state
unsigned int    gpio[4] = {_LED_0, _LED_1, _LED_2, _LED_3}; // map LEDs to gpio(s)

/********** support functions *******************************************************************/
void dispdat(void) {
	int 		channel;

	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		if (channel < 10)
			printf("  %i  - %i\n", channel, ipc_dat.c_dat[channel].c_state );
		else
			printf("  %i - %i\n", channel, ipc_dat.c_dat[channel].c_state );
	}
	return;
}

void update_relays(_tm *tm, IPC_DAT *sm) {

	int 				key;
	uint32_t			*srec_ptr;
	int 				state;
	int 				channel;

	key =  make_key(tm->tm_hour, tm->tm_min);									// generate key
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		srec_ptr = find_schedule_record(sm->sch[tm->tm_wday][channel], key);	// get a pointer to record matching key
		state =  get_s(*srec_ptr);												// extract state from schedule record
		sm->c_dat[channel].c_state = state;
	}
	printf("  %02i:%02i:%02i  %02i/%02i/%02i  dow %i\n",
	       tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon, tm->tm_mday, tm->tm_year, tm->tm_wday);
	return;
}


int main(void) {

	int 	h_min;
	int 	i;

	printf("\n  **** daemon active 0.2 ****\n\n");
	for (i = 0; i < 4; i++) 
		printf("    <%i>\n", gpio[i]);

	/********** initializations *******************************************************************/

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus
	/* setup shared memory */
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	memcpy(&ipc_dat, data, sizeof(ipc_dat));  	// move shared memory data to local structure
	/* setup semaphores */
	ipc_init_sem();
	printf("setting up gpios\n");
	/* setup gpio controlled leds */
	// for (i = 0; i < 4; i++) {
	// 	led_fd[i] = led_open(gpio[i]);
	// 	printf("file descriptor %i for led %i\n",led_fd[i],i );
	// 	led_set(led_fd[i], _OFF);
	// 	led_state[i] = _OFF;
	// }

	led_write(66,_ON);
	printf("got file descriptors\n");
	led_state[0] = _ON;
	led_set(led_fd[0], _ON);

	/********** main loop *******************************************************************/
	printf("starting main loop\n");
	while (1) {
		memcpy(&ipc_dat, data, sizeof(ipc_dat));  		// move shared memory data to local structure
		if (ipc_dat.force_update) {
			ipc_dat.force_update = 0;
			// wait for a lock on shared memory
			memcpy(data, &ipc_dat, sizeof(ipc_dat));  	// move local structure to shared memory
			// unlock shared memory
			update_relays(&tm, &ipc_dat);
			dispdat();
		}
		else {
			get_tm(rtc, &tm);
			if (h_min != tm.tm_min) {					// see if we are on a new minute
				h_min = tm.tm_min;
				update_relays(&tm, &ipc_dat);
				dispdat();
			}
		}

		/* cycle leds */
		for (i = 0; i < 4; i++) {
			if (led_state[i] == 1) {
				led_state[i] = 0;
				led_set(led_fd[i], _OFF);

				if (i < 3) {
					led_state[i + 1]  = 1;
					led_set(led_fd[i + 1] , _ON);
				}
				else {
					led_state[0] = 1;
					led_set(led_fd[0], _ON);
				}
				break;
			}
		}
		sleep(1);

	}

	return 0;
}