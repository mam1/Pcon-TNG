#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_


#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include "Pcon.h"

/* time data structure */
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

/* application configuration data */
typedef struct {
    int         major_version;
    int         minor_version;
    int         minor_revision;
    int 		channels;
    int 		sensors;
    int 		commands;
    int 		states;
} _CONFIG_DAT;

/* channel data */
typedef struct {
	int 		time_on; 	// accumulated minutes of on time for channel
	int			on_sec;		//on cycle in seconds
	int 		off_sec;	//off cycle in seconds
	int 		c_mode; 	//Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
	int 		c_state;	//0 = off, 1 = on
	int 		sensor_id; // sensor id of sensor associated with the channel
	char 		name[_CHANNEL_NAME_SIZE];
} _CHN_DAT;

/* schedule table data structures */
typedef struct {
	int			key;
	int 		state;
	int 		temp;
	int 		humid;
} _S_REC;				// format of schedule record

typedef struct {
	_S_REC 		rec[_MAX_SCHEDULE_RECS];
	int 		rcnt;
} _S_CHAN;				// combine schedule records into a schedule for a channel

typedef struct {	
	_S_CHAN 	schedule[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];	
} _S_TAB; 

/* schedule template */
typedef struct {
	char 			name[_SCHEDULE_NAME_SIZE];
	_S_CHAN			temp_chan_sch;
} _TMPL_DAT;
				// combine channel schedu


typedef struct {
	__TMPL_DAT 		t_lib[_MAX_TEMPLATES];
} _T_TAB

/* structure of system file record */
typedef struct {
	_CONFIG_DAT 	config;							// system configuration	
  	_S_TAB  		sys_sch;						// system schedule
  	_T_TAB 			sys_tem;						// schedule template library
  	_CHN_DAT     	c_data[_NUMBER_OF_CHANNELS];	// persistent channel data
    _TMPL_DAT		t_data[_MAX_TEMPLATES];			// schedule template library
    int 			schlib_index;					// points to the next available template record (maybe)

} _SYS_DAT2;

/* ipc data - memory mapped */
typedef	struct {
	int				force_update;	// force daemon to update channel
	_SYS_DAT2		sys_data;			// active schedule
    struct {
    	int 		sensor_id;		// sensor used for controling chanel
    	int 		c_state;		// 0 = off, 1 = on	
		int 		c_mode; 		// Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
		int			on_sec ;		// on cycle in seconds
		int 		off_sec;		// off cycle in seconds    
	} c_dat[_NUMBER_OF_CHANNELS];
	struct{
		int			temp;
		int			humidity;
		_tm 		ts;
	} s_dat[_NUMBER_OF_SENSORS];
} _IPC_DAT;

/* cmd_fsm control block */
typedef struct {
	int				state;
	int 			p_state;
	char 			token[_MAX_TOKEN_SIZE];
	int				token_type;
	int				token_value;
	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];

	_S_TAB        	w_sch,*w_sch_ptr;
	_S_CHAN         w_schedule;
	_SYS_DAT2		*sdat_ptr;

	int             w_channel;                      //working channel number
	int             w_schedule_name[_SCHEDULE_NAME_SIZE];
	int             w_schedule_number;
	int             w_minutes;
	char            w_minutes_str[4];
	int             w_hours;
	char            w_hours_str[4];
	int				w_srec_state;
	int 			w_state;
	int 			w_day;
	int 			w_temp;
	int 			w_humid;
	int 			w_state;
	int 			w_sensor_id;
	int				w_template_num;
	int				w_template_index;
} _CMD_FSM_CB;

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/

/* channel data */
typedef struct {
	int 		time_on; 	// accumulated minutes of on time for channel
	int			on_sec;		//on cycle in seconds
	int 		off_sec;	//off cycle in seconds
	int 		c_mode; 	//Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
	int 		c_state;	//0 = off, 1 = on
	int 		sensor_id; // sensor id of sensor associated with the channel
	char 		name[_CHANNEL_NAME_SIZE];
} CHN_DAT;

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
// typedef	struct {
//     int         major_version;
// } SYS_DAT;

//     int         minor_version;
//     int         minor_revision;	

//     // int 		template_id[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];			// map each (day,channel) to a template id
//     int 		schlib_index;												// points to the next available record (maybe)
//     uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
//     uint32_t	*sch_ptr;   												// pointer to system schedule
//     CHN_DAT     c_data[_NUMBER_OF_CHANNELS];								// channel persistent data
//     TMPL_DAT	s_data[_MAX_SCHLIB_SCH];									// schedule template library

/* cmd_fsm control block */
// typedef struct {
// 	int				state;
// 	int 			p_state;
// 	char 			token[_MAX_TOKEN_SIZE];
// 	int				token_type;
// 	int				token_value;
// 	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];
// 	uint32_t        w_sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];
// 	uint32_t		*w_sch_ptr;
// 	int             w_channel;                      //working channel number
// 	int             w_schedule_name[_SCHEDULE_NAME_SIZE];
// 	int             w_schedule_number;
// 	uint32_t        w_schedule[_SCHEDULE_SIZE];
// 	int             w_minutes;
// 	char            w_minutes_str[4];
// 	int             w_hours;
// 	char            w_hours_str[4];
// 	int				w_srec_state;
// 	int				w_template_index;
// 	int 			w_day;
// 	int 			w_temp;
// 	int 			w_humid;
// 	int 			w_sensor_id;
// 	int				w_template_num;
// 	SYS_DAT			*sdat_ptr;
// } CMD_FSM_CB;

typedef struct sembuf SEMBUF;  


/* ipc data - memory mapped */
// typedef	struct {
// 	// uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
// 	uint32_t    sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];	// system schedule
// 	int			force_update;		// force daemon to update channel
//     struct {
//     	int 		sensor_id;		// sensor used for controling chanel
//     	int 		c_state;		// 0 = off, 1 = on	
// 		int 		c_mode; 		// Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
// 		int			on_sec ;		// on cycle in seconds
// 		int 		off_sec;		// off cycle in seconds    
// 	} c_dat[_NUMBER_OF_CHANNELS];
// 	struct{
// 		int			temp;
// 		int			humidity;
// 		_tm 		ts;
// 	} s_dat[_NUMBER_OF_SENSORS];
// } IPC_DAT;


// typedef struct {
// 	int		key;
// 	int 	state;
// 	int 	temp;
// 	int 	humid;
// } _S_REC;

// typedef struct {
// 	_S_REC 	rec[_MAX_SCHEDULE_RECS];
// 	int 	rcnt;
// 	int 	sensor_id;
// } _S_DAY;

// typedef struct {	
// 	_S_DAY 	schedule[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];	
// } _SCH;

/* action routine definitions */
typedef int (*CMD_ACTION_PTR)(_CMD_FSM_CB *);
typedef int (*CHAR_ACTION_PTR)(char *);

#endif
