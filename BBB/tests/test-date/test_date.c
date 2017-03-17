#include <time.h>
#include <stdio.h>

#include <"PCF8563.h">

int main(void)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	get
	printf("RTC: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


/* set PCF8563  */
	int         rtc;

	/* Open the i2c-0 bus */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);

	/* read the clock */
	set_tm(rtc, &tm);
	close(rtc);

	c_2(cb);

	/* build prompt */
	c_34(cb);
	return 0;
}
