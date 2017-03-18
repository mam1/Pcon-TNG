/*
	set_rtc.c

	this code sets the PCF8563 real time clock
	to the local time fro m the systesm

*/

#include <time.h>
#include <stdio.h>
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include "typedefs.h"
#include "PCF8563.h"
// #include "/Users/mam1/Git/Pcon-TNG/BBB/include/typedefs.h"
// #include "/Users/mam1/Git/Pcon-TNG/BBB/include/PCF8563.h"
int main(void)
{
	time_t 		t;
	int         rtc;
	_tm 		rtc_time;

	/* get the system time */
	t = time(NULL);
    if (t == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        return 1;
    }
	struct tm tm = *localtime(&t);

	/* Open the i2c-0 bus */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);

	/* set the rtc to system clock */
	rtc_time.tm_sec = tm.tm_sec; 
    rtc_time.tm_min = tm.tm_min; 
    rtc_time.tm_hour = tm.tm_hour; 
    rtc_time.tm_mday = tm.tm_mday; 
    rtc_time.tm_wday = tm.tm_wday;
    rtc_time.tm_mon = tm.tm_mon + 1; 
    rtc_time.tm_year = tm.tm_year + 1900; 
    set_tm(rtc,&rtc_time);
	close_tm(rtc);

	return 0;
}
