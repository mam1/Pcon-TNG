
/*
 * In the routines that deal directly with the pcf8563 hardware, we use
 * rtc_time -- month 0-11, hour 0-23, yr = calendar year-epoch.
 */

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

// This function loads the time date structure
// from the PCF8563 register buffer
int get_tm(int rtc, _tm *tm){
  uint8_t   reg_buf[PCF8563_REGS];

  // select register to start read
  reg_buf[0] = SEC_REG;
  if (write(rtc,reg_buf,1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("Failed to write to the i2c bus.\n");
    printf("\n\n");
  }
  // read registers
  if (read(rtc,reg_buf,7) != 7) {
    /* ERROR HANDLING: i2c transaction failed */
    printf("Failed to read from the i2c bus: %s.\n", strerror(errno));
    printf("\n\n");
    } 
  else {
    tm->tm_sec = BCDToDecimal(reg_buf[0] & 0x7f);
    tm->tm_min = BCDToDecimal(reg_buf[1] & 0x7f);
    tm->tm_hour = BCDToDecimal(reg_buf[2] & 0x3f);
    tm->tm_mday = BCDToDecimal(reg_buf[3] & 0x3f);
    tm->tm_wday = reg_buf[4] & 0x7;
    tm->tm_mon = BCDToDecimal(reg_buf[5] & 0x1f) + 1;    /* rtc mn 1-12 */
    tm->tm_year = BCDToDecimal(reg_buf[6]) + 2000;
    // snprintf(&tm->tm_stamp[0], sizeof(tm->tm_stamp), "%4d%2d%2d%2d%2d",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min);
    // for(i=0;i<15;i++)
    // 	if(tm->tm_stamp[i] ==' ')
    // 		tm->tm_stamp[i] = '0';
  }
  return 0;

  return 0;
}

// This function loads the values in the date structure
// into the PCF8563.  

int set_tm(int rtc,_tm *tm){
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
  reg_buf[6] = DecimalToBCD((uint8_t)tm->tm_mon - 1);
  reg_buf[7] = DecimalToBCD((uint8_t)tm->tm_year - 2000);

  printf("  set_time: reg_buf[4] = %i\n\r",reg_buf[4]);
  printf("  set_time: BCDToDecimal(reg_buf[4] & 0x1f = %i\n\r",BCDToDecimal(reg_buf[4] & 0x1f));


  /* the buffer to the PCF8563 */
  if(write(rtc,reg_buf,8) != 8){
      printf("Failed to write to the i2c bus.\n");
    printf("\n\n");
  }

  return 0;
}