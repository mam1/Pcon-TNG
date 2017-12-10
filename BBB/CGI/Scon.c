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
#include <math.h>
#include <time.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "shared.h"
#include "bitlit.h"
#include "PCF8563.h"
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
_IPC_DAT        ipc_dat, *ipc_ptr;              // ipc data
char           	ipc_file[] = {_IPC_FILE_NAME};  // name of ipc file
void           	*data;                      	// pointer to ipc data
int            	fd;                        		// file descriptor for ipc data file
// int         	rtc;							// file descriptor for PCF8563 RTC
// _tm         	tm;								// time date structure						
time_t 			t;								// unix standate time structure
struct tm 		tm; 
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

float new_strtof(char* const ostr, char** endptr, unsigned char base)
{
    char* str = (char*)malloc(strlen(ostr) + 1);
    strcpy(str, ostr);
    const char* dot = ".";

    /* I do not validate any input here, nor do I do anything with endptr */      //Let's assume input of 101.1101, null, 2 (binary)
    char *cbefore_the_dot = strtok(str, dot); //Will be 101
    char *cafter_the_dot = strtok(NULL, dot); //Will be 0101

    float f = (float)strtol (cbefore_the_dot, 0, base); //Base would be 2 = binary. This would be 101 in decimal which is 5
    int i, sign = (str[0] == '-'? -1 : 1);
    char n[2] = { 0 }; //will be just for a digit at a time

    for(i = 0 ; cafter_the_dot[i] ; i++) //iterating the fraction string
    {
        n[0] = cafter_the_dot[i];
        f += strtol(n, 0, base) * pow(base, -(i + 1)) * sign; //converting the fraction part
    }

    free(str);
    return f;
}

/********** main routine ************************************************************************/
int main(void) {
	FILE 			*cgi_data;					// sensor data file
	FILE 			*cgi_log;					// cgi log
	char 			*s_num, *s_temp, *s_humid;
	long 			l_num;
	float 			l_temp, l_humid;
	// char 			file_name[_FILE_NAME_SIZE];
	// char           	sensor_log_file[] = {_SENSOR_DATA_FILE_NAME};   			// name of sensor log file

	char 			*eptr;

	_SEN_DAT_REC 		buffer;

	// int 			ipc;
	// int 			bkup;

	printf("Content-type: text/html\n\n");
	printf("\n  **** cgi %i.%i.%i active ****\n\r",_MAJOR_VERSION_Scon,_MINOR_VERSION_Scon,_MINOR_REVISION_Scon);
		// printf("\n  **** cgi active %s.%s.%s ****\n\r",_MAJOR_VERSION_Scon,_MINOR_VERSION_Scon,_MINOR_REVISION_Scon);


	/********** initializations *******************************************************************/

	// /* setup PCF8563 RTC */
	// rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus

	/* open files */

	cgi_log = fopen(_CGI_LOG_FILE_NAME,"ab");
	if(cgi_log == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _CGI_LOG_FILE_NAME);
		return 1;
	}

	cgi_data = fopen(_SENSOR_DATA_FILE_NAME,"a");
	if(cgi_data == NULL){
		sleep (1000);
		cgi_data = fopen(_SENSOR_DATA_FILE_NAME,"a");
		if(cgi_data == NULL){
			printf("  Error: %d (%s)\n", errno, strerror(errno));
			printf("    attempting to open %s\n\n application terminated\n\n", _SENSOR_DATA_FILE_NAME);
			(void) fprintf(cgi_log, "*** error attempting to open %s\n\n application terminated\n\n", _SENSOR_DATA_FILE_NAME);
			return 1;
		}
	}
	

	printf("%s"," CGI: files opened\n\r" );

	/* setup shared memory */
	ipc_sem_init();
	semid = ipc_sem_id(skey);					// get semaphore id
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = (_IPC_DAT *)data;					// overlay ipc data structure on shared memory
    ipc_sem_free(semid, &sb);                   // free lock on shared memory

    printf("%s"," CGI: shared memory setup\n\r" );

	/*********** start main process *******************************************************************/

	/* parce the query string for values sent by a ESP8266 */
	s_num = cgigetval("snesor");
	s_temp = cgigetval("temp");
	s_humid = cgigetval("humid");

	if(s_num== NULL || s_temp==NULL || s_humid==NULL){
		printf("*** error Scon passed null valuse from ESP8266\n\r");
		(void) fprintf(cgi_log, "*** error Scon passed null valuse from ESP8266\n\r");
		return 1;
	}


	l_num = strtol(s_num, &eptr, 10);
	if (l_num == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		(void) fprintf(cgi_log, "*** Conversion error occurred: %d", errno);
		exit(0);
	}
	if ((l_num < 0) && (*s_num > 99))
	{
		printf("*** sensor id out of range: %d", errno);
		(void) fprintf(cgi_log, "*** sensor id out of range: %d", errno);
		exit(0);
	}

	l_temp = new_strtof(s_temp, &eptr, 10);
	if (l_temp == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		(void) fprintf(cgi_log, "*** Conversion error occurred: %d", errno);
		exit(0);
	}


	l_humid = new_strtof(s_humid, &eptr, 10);
	if (l_humid == 0)
	{
		printf("*** Conversion error occurred: %d", errno);
		(void) fprintf(cgi_log, "*** Conversion error occurred: %d", errno);
		exit(0);
	}

	/* move sensor data to shared memory */
	ipc_sem_lock(semid, &sb);							// wait for a lock on shared memory

	/* get the system time */
	ipc_ptr->s_dat[(int)l_num].ts = time(NULL);
    if (ipc_ptr->s_dat[(int)l_num].ts == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        (void) fprintf(cgi_log, "Failure to obtain the current time.\n");
        return 1;
	}
	ipc_ptr->s_dat[(int)l_num].sensor_id = (int)l_num;
	ipc_ptr->s_dat[(int)l_num].temp = l_temp;
	ipc_ptr->s_dat[(int)l_num].humidity = l_humid;
	printf("%s"," CGI: sensor data moved to  shared memory\n\r" );

	/* log sensor data */
	buffer.ts = ipc_ptr->s_dat[(int)l_num].ts;
	buffer.sensor_id = ipc_ptr->s_dat[(int)l_num].sensor_id;
	buffer.temp = ipc_ptr->s_dat[(int)l_num].temp;
	buffer.humidity = ipc_ptr->s_dat[(int)l_num].humidity;
	ipc_sem_free(semid, &sb);							// free lock on shared memory

	if(fwrite(&buffer, sizeof(buffer), 1, cgi_data) != 1)
	{
		printf("*** error writing to %s\n", _SENSOR_DATA_FILE_NAME);
		(void) fprintf(cgi_log, "Failure to obtain the current time.\n");
	}
	else
	{
		/* get the system time */
		tm = *localtime(&buffer.ts);
		printf(" CGI: sensor %i, %i:%i:%i,  %i/%i/%i,  temp %0.2f,  humidity %0.2f\n\r",
			buffer.sensor_id, 
			tm.tm_hour, 
			tm.tm_min, 
			tm.tm_sec, 
			tm.tm_mon + 1, 
			tm.tm_mday, 
			tm.tm_year  + 1900,
			buffer.temp,
			buffer.humidity);
		printf(" CGI: data written to %s\n\r", _SENSOR_DATA_FILE_NAME);
	}
	fclose(cgi_data);
	fclose(cgi_log);
	printf(" CGI: normal termination\n\n");
	return 0;
}