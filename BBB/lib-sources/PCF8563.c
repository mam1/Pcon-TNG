
/*
 * In the routines that deal directly with the pcf8563 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch.
 */

/* PCF8563 version info */
#define _MAJOR_VERSION    10
#define _MINOR_VERSION    4
#define _MINOR_REVISION   0

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>    //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <time.h>

#include "PCF8563.h"

// Convert an 8 bit binary value to an 8 bit BCD value
static uint8_t  BCDToDecimal(uint8_t  bcdByte){ 
  return (((bcdByte & 0xF0) >> 4) * 10) + (bcdByte & 0x0F);
}
 
// Convert an 8 bit BCD value to an 8 bit binary value
static uint8_t DecimalToBCD(uint8_t decimalByte) { 
  return (((decimalByte / 10) << 4) | (decimalByte % 10));
}

// open the i2c buss connected to the PCF8563
int open_tm(char *filename, uint8_t addr){
  int     fn;

  if ((fn = open(filename, O_RDWR)) < 0) {
      /* ERROR HANDLING: you can check errno to see what went wrong */
      perror("Failed to open the i2c bus");
      exit(1);
  }

  if (ioctl(fn, I2C_SLAVE, addr) < 0) {
      printf("Failed to acquire bus access and/or talk to slave.\n");
      /* ERROR HANDLING; you can check errno to see what went wrong */
      exit(1);
  }
  return fn;
}

void close_tm(int f){
	close(f);
	return;
}

// This function loads the time date frome the system time
void get_tm(_tm *myt){
	time_t 			t;
	struct tm 		st;

	t = time(NULL);
    if (t == ((time_t)-1))
    {
        fprintf(stderr, "Failure to obtain the current time.\n");
        return;
    }
	st = *localtime(&t);

	myt->tm_sec = st.tm_sec; 
    myt->tm_min = st.tm_min; 
    myt->tm_hour = st.tm_hour; 
    myt->tm_mday = st.tm_mday; 
    myt->tm_wday = st.tm_wday;
    myt->tm_mon = st.tm_mon + 1; 
    myt->tm_year = st.tm_year + 1900; 

  return;
}

// This function loads the values in the date structure
// into the PCF8563.  

void set_tm(int rtc,_tm *tm){
  uint8_t   reg_buf[PCF8563_REGS];

  printf("  set_time: tm->tm_mday = %i\n\r",tm->tm_mday);

  /* start register */
  reg_buf[0] = SEC_REG;               
  /* time date registers */
  reg_buf[1] = DecimalToBCD((uint8_t)tm->tm_sec);
  reg_buf[2] = DecimalToBCD((uint8_t)tm->tm_min);
  reg_buf[3] = DecimalToBCD((uint8_t)tm->tm_hour);

  reg_buf[4] = DecimalToBCD((uint8_t)tm->tm_mday);
 
  reg_buf[5] = tm->tm_wday;
  reg_buf[6] = DecimalToBCD((uint8_t)tm->tm_mon);
  reg_buf[7] = DecimalToBCD((uint8_t)tm->tm_year - 2000);

  printf("  set_time: reg_buf[4] = %i\n\r",reg_buf[4]);
  printf("  set_time: BCDToDecimal(reg_buf[4] & 0x1f = %i\n\r",BCDToDecimal(reg_buf[4] & 0x1f));


  /* the buffer to the PCF8563 */
  if(write(rtc,reg_buf,8) != 8){
      printf("Failed to write to the i2c bus.\n");
    printf("\n\n");
  }

  return;
}