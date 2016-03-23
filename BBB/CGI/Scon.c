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

		if (p2 == NULL || p3 != NULL && p2 > p3)
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
	FILE 			*cgi_data;					// cgi log file handle
	char 			*s_num, *s_temp, *s_humid;
	long 			l_num, l_temp, l_humid;
	char 			cgi_file_name[_FILE_NAME_SIZE];
	char 			*eptr;

	printf("Content-type: text/html\n\n");
	printf("\n  **** cgi active 0.10 ****\n\r");
	// printf("Status: 200 OK\n");
	// printf("%s = <%s>\n\r", "QUERY_STRING", getenv("QUERY_STRING"));

	/********** initializations *******************************************************************/
	// printf("  starting initializations\n");

	/* setup PCF8563 RTC */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus

	/* build cgi file name */

	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (IPC_DAT *)data;					// overlay ipc data structure on shared memory
	printf("  %s copied to shared memory\n", ipc_file);

	/*********** start main process *******************************************************************/

	/* parce the query string for values sent by a ESP8266 */
	s_num = cgigetval("snesor");
	s_temp = cgigetval("temp");
	s_humid = cgigetval("humid");
	l_num = strtol(s_num, &eptr, 10);
	if (l_num == 0)
	{
		printf("Conversion error occurred: %d", errno);
		exit(0);
	}
	l_temp = strtol(s_temp, &eptr, 10);
	if (l_temp == 0)
	{
		printf("Conversion error occurred: %d", errno);
		exit(0);
	}
	l_humid = strtol(s_humid, &eptr, 10);
	if (l_humid == 0)
	{
		printf("Conversion error occurred: %d", errno);
		exit(0);
	}
	printf("  sensor %i, temp %i, humidity %i\n", (int)l_num, (int)l_temp, (int)l_humid);
	snprintf(cgi_file_name, _FILE_NAME_SIZE, "%s%i%s", _CGI_DATA_FILE_PREFIX, (int)l_num, _CGI_DATA_FILE_SUFIX);
	printf("\n******* file name <%s>\n",cgi_file_name);

	/* move sensor data to shared memory */
	get_tm(rtc, &(ipc_ptr->s_dat[l_num].ts));				// read the clock
	ipc_ptr->s_dat[l_num].temp = (int)l_temp;
	ipc_ptr->s_dat[l_num].humidity = (int)l_humid;
	printf("  sensor data copied to shared memory\n");
	ipc_ptr->force_update = 1;
	ipc_sem_free(semid, &sb);		// free lock on shared memory

	/* log sensor data */
	cgi_data = fopen(_CGI_DATA_FILE, "a");
	if (cgi_data == NULL) {
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _CGI_DATA_FILE);
		return 1;
	}
	printf("  %s opened\n", _CGI_DATA_FILE);
	printf("  write buffer size %i\n", sizeof(ipc_ptr->s_dat[l_num]));

	if (fwrite(&ipc_ptr->s_dat[l_num], sizeof(ipc_ptr->s_dat[l_num]), 1, cgi_data) != 1) {
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to append data to %s\n\n application terminated\n\n", _CGI_DATA_FILE);
		return 1;
	}
	printf("  data for sensor %i appended to %s\n", l_num, _CGI_DATA_FILE);
	printf("  dow = %i\n", ipc_ptr->s_dat[l_num].ts.tm_wday);

	fclose(cgi_data);
	printf("\nnormal termination\n\n");
	return 0;
}