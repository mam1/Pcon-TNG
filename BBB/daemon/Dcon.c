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
#include "trace.h"



/*********************** globals **************************/

#ifdef _TRACE
char			trace_buf[128];
#endif
int             	trace_flag;                   	//trace file is active

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

void update_relays(_tm *tm, IPC_DAT *sm) {

	int 				key;
	uint32_t			*s_ptr, *r_ptr;
	int 				state;
	int 				channel;

	
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++){
		
		switch(sm->c_dat[channel].c_mode){
			case 0:	// manual
				state = sm->c_dat[channel].c_state;
				break;
			case 1:	// time
				key =  make_key(tm->tm_hour, tm->tm_min);							// generate key
				s_ptr = get_schedule(((uint32_t *)sm->sch),tm->tm_wday,channel); 	// get a pointer to schedule for (day,channel)
				printf("got s_ptr\n"); 
				r_ptr = find_schedule_record(s_ptr,key);  							// search schedule for record with key match, return pointer to record or NULL	
				printf("got r_ptr\n");
				state =  get_s(*r_ptr);												// extract state from schedule record
			 	printf("got state\n");
			 	sm->c_dat[channel].c_state = state;
				break;
			case 2:	// time & sensor
				printf("*** error mode set to <2>\n");
				break;		
			case 3:	// cycle
				printf("*** error mode set to <3>\n");
				break;
			default: // error
				printf("*** error mode set to <%i>\n",sm->c_dat[channel].c_mode);
		}
#ifdef _TRACE
			sprintf(trace_buf, "    Dcon:update_relays:  relay %i set to %i\n", channel, state);
			strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
		printf("    relay %i set to %i\n", channel, state);
	}

	// printf("  %02i:%02i:%02i  %02i/%02i/%02i  dow %i\n",
	//        tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_mon, tm->tm_mday, tm->tm_year, tm->tm_wday);
	return;
}

int main(void) {

	int 	h_min;
	int 	toggle;

	printf("\n  **** daemon active 0.4 ****\n\n");

	/********** initializations *******************************************************************/

		/************************* setup trace *******************************/
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active,");
		if(trace_on(_TRACE_FILE_NAME,&trace_flag)){
			printf("trace_on returned error\n");
			trace_flag = false;
		}
	}
	if (trace_flag == false)
		printf(" program trace disabled\n");

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus
	/* setup shared memory */
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = data;								// overlay ipc data structure on shared memory
	// memcpy(ipc_ptr, data, sizeof(ipc_dat));  	// move shared memory data to local structure
	printf("\n  force_update <%i>\n\n", ipc_ptr->force_update);
	
	/* setup semaphores */
	// ipc_init_sem();

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
		if (ipc_ptr->force_update == 1) {
			ipc_ptr->force_update = 0;
			printf("  *** update forced\n");
			// wait for a lock on shared memory
//			memcpy(data, ipc_ptr, sizeof(ipc_dat));  	// move local structure to shared memory
			// unlock shared memory
			update_relays(&tm, ipc_ptr);
			dispdat();
		}
		else {
			get_tm(rtc, &tm);
			if (h_min != tm.tm_min) {					// see if we are on a new minute
				h_min = tm.tm_min;
				printf("  *** update triggered by time\n");
				update_relays(&tm, ipc_ptr);
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