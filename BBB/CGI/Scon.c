/********************************************************************/
/*	Scon.c - cgi that updates sensor values in shared memory        */
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
#include "typedefs.h"

// #define _TRACE

// /*********************** globals **************************/
// #ifdef _TRACE
// char			trace_buf[128];
// #endif
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

/********** main routine ************************************************************************/
int main(void) {
	FILE 			*cgi_data;
//	CGI_DAT 		write_buffer;
	// int 			ret;
//	_tm 			tm;
	int         	rtc;		// file name for real time clock
	int 			sensor_number;
	int 			sensor_value;


	printf("\n  **** cgi active 0.0 ****\n\n");

	/********** initializations *******************************************************************/
	printf("  starting initializations\n");

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus

	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (IPC_DAT *)data;					// overlay ipc data structure on shared memory
	printf("  %s copied to shared memory\n",ipc_file);
	
	/* move sensor data to shared memory */
	get_tm(rtc, &(ipc_ptr->s_dat[sensor_number].ts));				// read the clock
	ipc_ptr->s_dat[sensor_number].temp = 66 * 10;
	ipc_ptr->s_dat[sensor_number].humidity = 45 * 10;
	printf("  sensor data copied to shared memory\n");
	ipc_ptr->force_update = 1;
	ipc_sem_free(semid, &sb);		// free lock on shared memory

	/* log sensor data */
	cgi_data = fopen(_CGI_DATA_FILE,"a");
	if(cgi_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _CGI_DATA_FILE);
		return 1;
	}
	printf("  %s opened\n",_CGI_DATA_FILE);
	printf("  write buffer size %i\n", sizeof(ipc_ptr->s_dat[sensor_number]));
	printf("%s\n",&ipc_ptr->s_dat[sensor_number]);

	if(fwrite(&ipc_ptr->s_dat[sensor_number], sizeof(ipc_ptr->s_dat[sensor_number]), 1, cgi_data) != 1){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to append data to %s\n\n application terminated\n\n", _CGI_DATA_FILE);
		return 1;
	}
	printf("  data appended to %s\n",_CGI_DATA_FILE);


	// #ifdef _TRACE
	// 	strace(_TRACE_FILE_NAME, "Scon: starting main loop", trace_flag);
	// #endif
	fclose(cgi_data);
	printf("\nnormal termination\n\n");
	return 0;
}