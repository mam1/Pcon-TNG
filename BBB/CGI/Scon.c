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
#include "BBBiolib.h"
#include "trace.h"
#include "typedefs.h"
#include "sys_dat.h"
#include "Scon.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *mode[4] = {"manual", "  time", "   t&s", " cycle"};

/********** globals *******************************************************************/
_IPC_DAT        ipc_dat, *ipc_ptr;                    		// ipc data
char           	ipc_file[] = {_IPC_FILE_NAME};   			// name of ipc file
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

char *unescstring(char *, int, char *, int);

char *cgigetval(char *fieldname)
{
	int fnamelen;
	char *p, *p2, *p3;
	int len1, len2;
	static char *querystring = NULL;
	if (querystring == NULL)
	{
		querystring = getenv("QUERY_STRING");
		if (querystring == NULL)
			return NULL;
	}

	if (fieldname == NULL)
		return NULL;

	fnamelen = strlen(fieldname);

	for (p = querystring; *p != '\0';)
	{
		p2 = strchr(p, '=');
		p3 = strchr(p, '&');
		if (p3 != NULL)
			len2 = p3 - p;
		else	len2 = strlen(p);

		if ((p2 == NULL || p3 != NULL) && p2 > p3)

		{
			/* no = present in this field */
			p3 += len2;
			continue;
		}
		len1 = p2 - p;

		if (len1 == fnamelen && strncmp(fieldname, p, len1) == 0)
		{
			/* found it */
			int retlen = len2 - len1 - 1;
			char *retbuf = malloc(retlen + 1);
			if (retbuf == NULL)
				return NULL;
			unescstring(p2 + 1, retlen, retbuf, retlen + 1);
			return retbuf;
		}

		p += len2;
		if (*p == '&')
			p++;
	}

	/* never found it */

	return NULL;
}

static int xctod(int);

char *unescstring(char *src, int srclen, char *dest, int destsize)
{
	char *endp = src + srclen;
	char *srcp;
	char *destp = dest;
	int nwrote = 0;

	for (srcp = src; srcp < endp; srcp++)
	{
		if (nwrote > destsize)
			return NULL;
		if (*srcp == '+')
			*destp++ = ' ';
		else if (*srcp == '%')
		{
			*destp++ = 16 * xctod(*(srcp + 1)) + xctod(*(srcp + 2));
			srcp += 2;
		}
		else	*destp++ = *srcp;
		nwrote++;
	}

	*destp = '\0';

	return dest;
}

static int xctod(int c)
{
	if (isdigit(c))
		return c - '0';
	else if (isupper(c))
		return c - 'A' + 10;
	else if (islower(c))
		return c - 'a' + 10;
	else	return 0;
}

/********** main routine ************************************************************************/
int main(void) {
	FILE 			*cgi_data;					// sensor data file
	FILE 			*cgi_log;					// cgi log
	char 			*s_num, *s_temp, *s_humid;
	long 			l_num, l_temp, l_humid;
	// char 			file_name[_FILE_NAME_SIZE];
	char           	sensor_log_file[] = {_SENSOR_LOG_FILE_NAME};   			// name of sensor log file

	char 			*eptr;
	struct{
		int 		sensor_id;
		int			temp;
		int			humidity;
		_tm 		ts;
	} buffer;

	// int 			ipc;
	// int 			bkup;

	printf("Content-type: text/html\n\n");
	printf("\n  **** cgi %i.%i.%i active ****\n\r",1,0,0);
		// printf("\n  **** cgi active %s.%s.%s ****\n\r",_MAJOR_VERSION_Scon,_MINOR_VERSION_Scon,_MINOR_REVISION_Scon);


	/********** initializations *******************************************************************/

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus

	/* open files */
	cgi_data = fopen(sensor_log_file,"a");
	if(cgi_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", sensor_log_file);
		return 1;
	}
	cgi_log = fopen(_CGI_LOG_FILE_NAME,"a");
	if(cgi_log == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _CGI_LOG_FILE_NAME);
		return 1;
	}

	// /* check for ipc file and ipc backup file */	
 //    if( access(_IPC_FILE_BACKUP_NAME, F_OK ) != -1 ){
 //        bkup = 1;
 //        fprintf(stderr, "%s\n"," ipc backup found" );
 //    }
 //    else{ 
 //        bkup = 0;
 //        fprintf(stderr, "%s\n"," ipc backup not found" );
 //    }
 //    if( access(_IPC_FILE_BACKUP_NAME, F_OK ) != -1 ){
 //        ipc = 1;
 //        fprintf(stderr, "%s\n"," ipc file found" );

 //    }
 //    else
 //        ipc = 0;
 //    }

	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (_IPC_DAT *)data;					// overlay ipc data structure on shared memory

	// if(ipc==0){
	// 	fprintf(stderr, "%s\n"," ipc file not found" );
	// 	fprintf(stderr, "%s\n"," new ipc file created and initialized" );
	// 	ipc_sem_lock(semid, &sb);                   // wait for a lock on shared memory
 //        ipc_ptr->sys_data.config.major_version = _MAJOR_VERSION_system;
 //        ipc_ptr->sys_data.config.minor_version = _MINOR_VERSION_system;
 //        ipc_ptr->sys_data.config.minor_revision = _MINOR_REVISION_system;
 //        ipc_ptr->sys_data.config.channels = _NUMBER_OF_CHANNELS;
 //        ipc_ptr->sys_data.config.sensors = _NUMBER_OF_SENSORS;
 //        ipc_ptr->sys_data.config.commands = _CMD_TOKENS;
 //        ipc_ptr->sys_data.config.states = _CMD_STATES;
 //        ipc_sem_free(semid, &sb);                   // free lock on shared memory
	// }


	/*********** start main process *******************************************************************/

	/* parce the query string for values sent by a ESP8266 */
	s_num = cgigetval("snesor");
	s_temp = cgigetval("temp");
	s_humid = cgigetval("humid");
	l_num = strtol(s_num, &eptr, 10);
	if (l_num == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		exit(0);
	}
	l_temp = strtol(s_temp, &eptr, 10);
	if (l_temp == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		exit(0);
	}
	l_humid = strtol(s_humid, &eptr, 10);
	if (l_humid == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		exit(0);
	}

	/* move sensor data to shared memory */
	ipc_sem_lock(semid, &sb);							// wait for a lock on shared memory
	get_tm(rtc, &(ipc_ptr->s_dat[(int)l_num].ts));		// read the clock
	ipc_ptr->s_dat[(int)l_num].sensor_id = (int)l_num;
	ipc_ptr->s_dat[(int)l_num].temp = (int)l_temp;
	ipc_ptr->s_dat[(int)l_num].humidity = (int)l_humid;
	ipc_sem_free(semid, &sb);							// free lock on shared memory


	/* log sensor data */
	// get_tm(rtc, &buffer.ts);
	buffer.ts = ipc_ptr->s_dat[(int)l_num].ts;
	buffer.sensor_id = ipc_ptr->s_dat[(int)l_num].sensor_id;
	buffer.temp = ipc_ptr->s_dat[(int)l_num].temp;
	buffer.humidity = ipc_ptr->s_dat[(int)l_num].humidity;

	if(fwrite(&buffer, sizeof(buffer), 1, cgi_data) != 1)
		printf("*** error writing to %s\n", sensor_log_file); 
	else 
		printf(" CGI: data logged to %s\n",sensor_log_file);
	fclose(cgi_data);
	fclose(cgi_log);
	printf(" CGI: normal termination\n\n");
	return 0;
}