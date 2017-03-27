#include <time.h>
#include <stdio.h>
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.

#include "/Users/mam1/Git/Pcon-TNG/BBB/include/typedefs.h"
#include "/Users/mam1/Git/Pcon-TNG/BBB/include/PCF8563.h"

int main(void)
{
	time_t 			t;
	struct tm 		tm;

	t = time(NULL);
    if (t == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        return 1;
    }
	tm = *localtime(&t);

	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

/* set PCF8563  */
	int         rtc;
	_tm 		rtc_time;

	/* Open the i2c-0 bus */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);

	/* read the rtc */
	get_tm(&rtc_time);
	printf("RTC: %d-%d-%d %d:%d:%d\n", rtc_time.tm_year, rtc_time.tm_mon, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);

	/* set the rtc to system clock */
	rtc_time.tm_sec = tm.tm_sec; 
    rtc_time.tm_min = tm.tm_min; 
    rtc_time.tm_hour = tm.tm_hour; 
    rtc_time.tm_mday = tm.tm_mday; 
    rtc_time.tm_wday = tm.tm_wday;
    rtc_time.tm_mon = tm.tm_mon + 1; 
    rtc_time.tm_year = tm.tm_year + 1900; 

    printf(" %02i:%02i:%02i  - %04i -  %02i/%02i/%02i\n\r",
	       rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec, rtc_time.tm_wday, rtc_time.tm_mon, rtc_time.tm_mday, rtc_time.tm_year);

    printf("clock reset\n");
    set_tm(rtc, &rtc_time);

    get_tm(&rtc_time);
	printf("RTC: %d-%d-%d % 2d:% 2d:% 2d\n", rtc_time.tm_year, rtc_time.tm_mon, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);


	close_tm(rtc);

	return 0;
}
