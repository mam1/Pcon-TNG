#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#include "Pcon.h"
// #include "cmd_fsm.h"

/* channel data */
typedef struct {
	int 		time_on; 	// accumulated minutes of on time for channel
	int			on_sec;		//on cycle in seconds
	int 		off_sec;	//off cycle in seconds
	char 		name[_CHANNEL_NAME_SIZE];
	int 		c_mode; 	//Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
	int 		c_state;	//0 = off, 1 = on
} CHN_DAT;

/* schedule templates */
typedef struct {
	char 			name[_SCHEDULE_NAME_SIZE];
	uint32_t		schedule[_SCHEDULE_SIZE];
} TMPL_DAT;

/* system data - saved and restored at restart */
typedef	struct {
    int         major_version;
    int         minor_version;
    int         minor_revision;	
    // int 		template_id[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];			// map each (day,channel) to a template id
    int 		schlib_index;												// points to the next available record (maybe)
    uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
    uint32_t	*sch_ptr;   												// pointer to system schedule
    CHN_DAT     c_data[_NUMBER_OF_CHANNELS];								// channel persistent data
    TMPL_DAT	s_data[_MAX_SCHLIB_SCH];									// schedule template library
} SYS_DAT;

/* cmd_fsm control block */
typedef struct {
	int				state;
	int 			p_state;
	char 			token[_MAX_TOKEN_SIZE];
	int				token_type;
	int				token_value;
	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];
	uint32_t        w_sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];
	uint32_t		*w_sch_ptr;
	int             w_channel;                      //working channel number
	int             w_schedule_name[_SCHEDULE_NAME_SIZE];
	int             w_schedule_number;
	uint32_t        w_schedule[_SCHEDULE_SIZE];
	int             w_minutes;
	char            w_minutes_str[4];
	int             w_hours;
	char            w_hours_str[4];
	int				w_srec_state;
	int				w_template_index;
	int 			w_day;
	int				w_template_num;
	SYS_DAT			*sdat_ptr;
} CMD_FSM_CB;

/* action routine definitions */
typedef int (*CMD_ACTION_PTR)(CMD_FSM_CB *);
typedef int (*CHAR_ACTION_PTR)(char *);

/************************************************************************************/
/************************************************************************************/

// /* control block pointer definitions */
// typedef volatile struct RTC_CB *RTC_CB_ADDR;
// typedef volatile struct DIO_CB *DIO_CB_ADDR;

// /* working schedule buffer definition */
// typedef struct {
// 	uint8_t b0;					//i2c byte 1 
// 	uint8_t b1;					//i2c byte 2
// 	uint8_t buf[_BYTES_PER_WORKING_SET];//current day schedules for all channels
// } WORKING_SCH_BUFFER;

// /* channel control array element definition */
// typedef struct {
// 	int time_on; // accumulated minutes of on time for channel
// 	char name[_CHANNEL_NAME_SIZE];
// 	uint8_t c_mode; //Control mode: 0-manual, 1-time, 2-time & sensor
// 	uint8_t state;  //Channel State: 0-off, 1-on
// } CHN_DAT;

// /* dio control block definition */
// typedef volatile struct {
// 	int rtc_lock;     	          //lock ID for rtc cog contorl block
// 	RTC_CB_ADDR rtc_cb_addr;              //address of the rtc cog control block
// 	WORKING_SCH_BUFFER wsb; //current day schedules for all channels + 2 bytes for i2c communication
// 	CHN_DAT cca[_NUMBER_OF_CHANNELS]; //channel control array
// 	int save_cca;                 //!= 0 cog request a save of the channel data
// 	uint8_t abt;           //!= 0 cog requests a system abort,value = error code
// } DIO_CB;

// /* rtc cog control block definition */
// typedef volatile struct {
// 	int dio_lock;       //lock ID for dio cog control block
// 	int rtc_lock;     	//lock ID for rtc cog contorl block
// 	DIO_CB_ADDR dio_cb_addr;    //address of the dio cog control block
// 	uint8_t update_sec;     //trigger update, 1=wait, 0=update 
// 	uint8_t update_day; 	//trigger update, 1=wait, 0=update 
// 	uint8_t set;            //!= 0, set clock
// 	uint8_t set_data[7];    //BCD, time and date to be loaded
// 	uint8_t sec;            //current second
// 	uint8_t min;            //current minute
// 	uint8_t hour;           //current hour
// 	uint8_t day;            //current day
// 	uint8_t month;          //current month
// 	uint8_t year;           //current year
// 	uint8_t dow;            //current day of the week
// 	uint8_t abt;           //!= 0 cog requests a system abort,value = error code
// } RTC_CB;

#endif
