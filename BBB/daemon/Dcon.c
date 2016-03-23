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
void bin_prnt_byte(uint8_t x)
{
   int n;

   for(n=0; n<8; n++)
   {
      if((x & 0x80) !=0)
         printf("1");
      else
         printf("0");
      if (n==3)
         printf(" "); /* insert a space between nybbles */
      x = x<<1;
   }
   printf("\n");
   return;
}

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

void send_ccb(uint8_t byte)         //send control byte to dio board
 {
    int         i;
 
    pin_low(8,_DIOB_DIN);  
    for(i=7;i>=0;i--)   //serialize and reverse bits 
    {
        if((1 << i) & byte)
            pin_high(8,_DIOB_DIN);  //send bit high
        else
            pin_low(8,_DIOB_DIN);   //send bit low
        /* send clock pluse */                
        pin_high(8,_DIOB_SCLK_IN);  
        pin_low(8,_DIOB_SCLK_IN);
    }   
    pin_high(8,_DIOB_LAT_RLY);       //set the LAT_RlY to high this will cause the HC595 to read the value from the shift register */     
    pin_low(8,_DIOB_LAT_RLY);        //done - ready for next write */ 
    return;
 }

void update_relays(_tm *tm, IPC_DAT *ipc_ptr) {

	int 				key;
	uint32_t			*s_ptr;		// *r_ptr;
	int 				state;
	int 				channel;
	uint8_t 			ccb;
	int 				gpio_index;

	static int 			pin[_NUMBER_OF_GPIOS] = {_PINS};
	static int 			header[_NUMBER_OF_GPIOS] = {_HEADERS};


	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	// printf("got the lock\n\n");
	
	/* set channel state based on channel mode */
	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++) {
		switch (ipc_ptr->c_dat[channel].c_mode) {
		case 0:	// manual
			state = ipc_ptr->c_dat[channel].c_state;
			break;
		case 1:	// time
			key =  make_key(tm->tm_hour, tm->tm_min);								// generate key
			s_ptr = get_schedule(((uint32_t *)ipc_ptr->sch), tm->tm_wday, channel); // get a pointer to the schedule for (day,channel)
			printf("  cmd:fsm testing schedule for day %i channel %i\n", tm->tm_wday, channel);
			// dump_sch(s_ptr);
			state =  test_sch(s_ptr, key);
			// printf("  cmd_fsm:  <%i> returned from test_sch\n", state);
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

#ifdef _TRACE 
	// printf("moving on\n"); 
	// dispdat();
	// printf("\nupdating relays\n");
#endif

	/* update on board relays channels 0-7 */    	
	for (channel = 0;channel < 8; channel++) {
		gpio_index = channel + 8;
		if (ipc_ptr->c_dat[channel].c_state){
			pin_high(header[gpio_index], pin[gpio_index]);
		}
		else{ 
			pin_low(header[gpio_index], pin[gpio_index]);
		}
	}
#ifdef _TRACE
	printf("\nchannels 0-7 set\n");
#endif
	/* update DBIO relays channels 8-15 */
	for (channel = 8; channel < 16; channel++) {
	    if(ipc_ptr->c_dat[channel].c_state)
	        ccb |= (1<<((channel - 8)));
	    else
	        ccb &= ~(1<<((channel - 8)));
	}
	send_ccb(ccb);         		// send a control byte to the DIOB 
#ifdef _TRACE
	printf("\nchannels 8-15 set\n");
	printf("control byte: ");
	bin_prnt_byte(ccb);
#endif
	ipc_sem_free(semid, &sb);	// free lock on shared memory
	return;

}

/* initialise gpio pins for iolib */
int init_gpio(void) {
	char 				command[120];
	int 				i;
	int 				gpios[_NUMBER_OF_GPIOS] = {_GPIOS};

	for (i = 0; i < _NUMBER_OF_GPIOS; i++) {
		sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpios[i], gpios[i] );
		printf("command <%s>\n", command);
		printf("system command %s returned %i\n", command, system(command));
	}

	return 0;
}

int main(void) {

	int 	h_min;
	int 	toggle;

	printf("\n  **** daemon active 0.12 ****\n\n");

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
#ifdef _TRACE
	printf("PCF8563 opened\n");
#endif
	/* setup shared memory */
	ipc_sem_init();								// setup semaphores
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (IPC_DAT *)data;					// overlay ipc data structure on shared memory
	ipc_ptr->force_update = 1;
	ipc_sem_free(semid, &sb);					// free lock on shared memory
#ifdef _TRACE
	printf("shared memory setup completed\n");
#endif
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
	printf("\n mapping DIOB serial header to gpio pins\n");
	iolib_setdir(8, _DIOB_DIN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_SCLK_IN, BBBIO_DIR_OUT);
	iolib_setdir(8, _DIOB_LAT_RLY, BBBIO_DIR_OUT);
	pin_low(8, _DIOB_SCLK_IN);
    pin_low(8, _DIOB_DIN);


/********** main loop *******************************************************************/

	printf("starting main loop\n");
	while (1) {

		if (ipc_ptr->force_update == 1) {
			ipc_ptr->force_update = 0;
			printf("\n*** update forced\n");
			update_relays(&tm, ipc_ptr);
			// dispdat();
		}
		else {
			get_tm(rtc, &tm);
			if (h_min != tm.tm_min) {					// see if we are on a new minute
				h_min = tm.tm_min;
				printf("\n*** update triggered by time:");
				printf("  %02i:%02i:%02i  %02i/%02i/%02i  dow %i\n",
				       tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mon, tm.tm_mday, tm.tm_year, tm.tm_wday);
				update_relays(&tm, ipc_ptr);
				// dispdat();
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