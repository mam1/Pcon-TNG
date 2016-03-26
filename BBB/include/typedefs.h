#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_


#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include "Pcon.h"

/* time date structure */
typedef struct {
    int tm_sec;         // seconds
    int tm_min;         // minutes
    int tm_hour;        // hours
    int tm_mday;        // day of the month
    int tm_mon;         // month
    int tm_year;        // year
    int tm_wday;        // day of the week
    int tm_yday;        // day in the year
    int tm_isdst;       // daylight saving time
} _tm;

/* channel data */
typedef struct {
	int 		time_on; 	// accumulated minutes of on time for channel
	int			on_sec;		// on cycle in seconds
	int 		off_sec;	// off cycle in seconds
	int 		c_mode; 	// Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
	int 		c_state;	// 0 = off, 1 = on
	int 		sensor_id;	// sensor id of sensor associated with the channel
	char 		name[_CHANNEL_NAME_SIZE];
} CHN_DAT;

/* schedule table data structures */
typedef struct {
	int		key;
	int 	state;
	int 	temp;
	int 	humid;
} _S_REC;			// format of schedule record

typedef struct {
	_S_REC 	rec[_MAX_SCHEDULE_RECS];
	int 	rcnt;
	// int 	sensor_id;
} _S_CHAN;				// combine schedule records into a schedule for a channel

typedef struct {	
	_S_CHAN 	schedule[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];	
} _S_TAB; 				// combine channel schedules into a table

/* schedule templates */
typedef struct {
	char 			name[_SCHEDULE_NAME_SIZE];
	uint32_t		schedule[_SCHEDULE_SIZE];
	_S_CHAN 		chan_sch;
} TMPL_DAT;

/*  data structures for indexing the schedule array */
typedef struct
 {
    uint32_t            rec[_MAX_SCHEDULE_RECS+1];
 } SCH;

typedef struct
 {
    SCH                 sch[_NUMBER_OF_CHANNELS];
 } DAY;

/* system data - saved and restored at restart */
typedef	struct {
    int         major_version;
    int         minor_version;
    int         minor_revision;	
    int 		schlib_index;												// points to the next available record (maybe)
    uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
  _S_TAB  		sch2;  
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
  _S_TAB  			w_sch2;
  _S_CHAN			w_schedule2[_MAX_SCHEDULE_RECS];
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
	int 			w_temp;
	int 			w_humid;
	int 			w_sensor_id;
	int				w_template_num;
	SYS_DAT			*sdat_ptr;
} CMD_FSM_CB;

typedef struct sembuf SEMBUF;  

/* ipc data - memory mapped */
typedef	struct {
	uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
  _S_TAB 			sch2;
	int			force_update;		// force daemon to update channel
	CHN_DAT 		c_dat[_NUMBER_OF_CHANNELS];
	struct{
		int			temp;
		int			humidity;
		_tm 		ts;
	} s_dat[_NUMBER_OF_SENSORS];
} IPC_DAT;

/************************************************************************/

/* action routine definitions */
typedef int (*CMD_ACTION_PTR)(CMD_FSM_CB *);
typedef int (*CHAR_ACTION_PTR)(char *);

#endif
