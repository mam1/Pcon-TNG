#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#include "Pcon.h"

/* action routines for the cmd fsm */
typedef int (*CMD_ACTION_PTR)(int, int *, char *);

/* control block pointer definitions */
typedef volatile struct RTC_CB *RTC_CB_ADDR;
typedef volatile struct DIO_CB *DIO_CB_ADDR;

/* working schedule buffer definition */
typedef struct {
	uint8_t b0;					//i2c byte 1 
	uint8_t b1;					//i2c byte 2
	uint8_t buf[_BYTES_PER_WORKING_SET];//current day schedules for all channels
} WORKING_SCH_BUFFER;

/* channel control array element definition */
typedef struct {
	int time_on; // accumulated minutes of on time for channel
	char name[_CHANNEL_NAME_SIZE];
	uint8_t c_mode; //Control mode: 0-manual, 1-time, 2-time & sensor
	uint8_t state;  //Channel State: 0-off, 1-on
} CCR;

/* dio control block definition */
typedef volatile struct {
	int rtc_lock;     	          //lock ID for rtc cog contorl block
	RTC_CB_ADDR rtc_cb_addr;              //address of the rtc cog control block
	WORKING_SCH_BUFFER wsb; //current day schedules for all channels + 2 bytes for i2c communication
	CCR cca[_NUMBER_OF_CHANNELS]; //channel control array
	int save_cca;                 //!= 0 cog request a save of the channel data
	uint8_t abt;           //!= 0 cog requests a system abort,value = error code
} DIO_CB;

/* rtc cog control block definition */
typedef volatile struct {
	int dio_lock;       //lock ID for dio cog control block
	int rtc_lock;     	//lock ID for rtc cog contorl block
	DIO_CB_ADDR dio_cb_addr;    //address of the dio cog control block
	uint8_t update_sec;     //trigger update, 1=wait, 0=update 
	uint8_t update_day; 	//trigger update, 1=wait, 0=update 
	uint8_t set;            //!= 0, set clock
	uint8_t set_data[7];    //BCD, time and date to be loaded
	uint8_t sec;            //current second
	uint8_t min;            //current minute
	uint8_t hour;           //current hour
	uint8_t day;            //current day
	uint8_t month;          //current month
	uint8_t year;           //current year
	uint8_t dow;            //current day of the week
	uint8_t abt;           //!= 0 cog requests a system abort,value = error code
} RTC_CB;

#endif
