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

typedef struct {
	int			temp;
	int			humidity;
	_tm 		ts;
} _SEN_DAT;

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
	char 			name[_TEMPLATE_NAME_SIZE];
	int 			rcnt;
	_S_REC			rec[_MAX_SCHEDULE_RECS];

} _TMPL_DAT;

typedef struct {
	_TMPL_DAT 		t_lib[_MAX_TEMPLATES];
} _T_TAB;

/* structure of system file record */
typedef struct {
	_CONFIG_DAT 	config;							// system configuration	
  	_S_TAB  		sys_sch;						// system schedule
  	_TMPL_DAT 		sys_tpl[_MAX_TEMPLATES];		// schedule template library
  	int 			schlib_index;					// points to the next available template record 
	 _CHN_DAT     	c_data[_NUMBER_OF_CHANNELS];	// persistent channel data
} _SYS_DAT2;

/* ipc data - memory mapped */
typedef	struct {
	int				force_update;				// force daemon to update relays
	_SYS_DAT2		sys_data;					// persistant system data
	_SEN_DAT		s_dat[_NUMBER_OF_SENSORS];  // current sensor values - set by cgi 
} _IPC_DAT;

/* cmd_fsm control block */
typedef struct {
	/* controled by fsm */
	int				state;
	int 			p_state;
	char 			token[_MAX_TOKEN_SIZE];
	int				token_type;
	int				token_value;
	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];
	/* working data */
	int             w_channel;                      		//working channel number
	int             w_minutes;
	char            w_minutes_str[4];
	int             w_hours;
	char            w_hours_str[4];
	int 			w_day;
	int				w_srec_state;
	int 			w_state;
	int 			w_temp;
	int 			w_humid;
	int 			w_sensor_id;
	_TMPL_DAT 		w_template_buffer;
	_S_TAB        	w_sch, *w_sch_ptr;
	_S_CHAN         w_tbuff;
	int      		w_schedule_number;
	int             w_schedule_name[_SCHEDULE_NAME_SIZE];
	int      		w_template_number;
	int             w_template_name[_TEMPLATE_NAME_SIZE];
	int				w_template_num;
	int				w_template_index;

	/* pointers */
	_IPC_DAT 		*ipc_ptr;			//pointer to shared memory
	_SYS_DAT2		*sys_ptr;			//pointer to system data in shared memory
	_S_TAB 			*sch_ptr;			//pointer to active schedule in shared memory


} _CMD_FSM_CB;

/* buffer for semaphores */
typedef struct sembuf SEMBUF;  


/* action routine definitions */
typedef int (*CMD_ACTION_PTR)(_CMD_FSM_CB *);
typedef int (*CHAR_ACTION_PTR)(char *);

#endif
