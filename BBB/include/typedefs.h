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

/* system configuration data */
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
	int 		time_on; 			// accumulated minutes of on time for channel
	int			on_sec;				// on cycle in seconds
	int 		off_sec;			// off cycle in seconds
	int 		mode; 				// control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
	int 		state;				// 0 = off, 1 = on
	int 		sensor_id;  		// sensor id of sensor associated with the channel
	int 		sensor_assigned;
	char 		name[_CHANNEL_NAME_SIZE];
} _CHN_DAT;

// typedef struct sl_node {
// 	char 				name[10];
// 	char 		 		definition[50];
// 	int 				id;
// 	struct sl_ node 	*next;
// } _SL_NODE;

/* sensor data */
typedef struct {
	int 		sensor_id;
	int 		active;
	char 		name[_SNAME_SIZE];
	char 		description[_SDESCRIPTION];
	int 		channel[_NUMBER_OF_CHANNELS];  	// channel assignments
	int 		channel_index;					// points to the next open assingment
	float		temp;							// current sensor reading
	float		humidity;						// current sensor reading
	_tm 		ts; 							// time and date of the reading
} _SEN_DAT;

/* sensor data log file */
typedef	struct{
		int 		sensor_id;
		float		temp;
		float		humidity;
		_tm 		ts;
} _SEN_DAT_REC;

/* schedule record */
typedef struct {
	int			key;
	int 		state;
	float 		temp;
	float 		humid;
} _S_REC;				

/* schedule  */
typedef struct {
	char 			name[_TEMPLATE_NAME_SIZE];
	int 			rcnt;
	_S_REC			rec[_MAX_SCHEDULE_RECS];

} _TMPL_DAT;

/* schedule table */
typedef struct {
	_TMPL_DAT 	sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];
} _S_TAB;

/* template library */
typedef struct {
	_TMPL_DAT 		t_lib[_MAX_TEMPLATES];
} _T_TAB;

/* structure of system file record */
typedef struct {
	_CONFIG_DAT 	config;							// system configuration
	_S_TAB  		sys_sch;						// system schedule
	_CHN_DAT     	c_data[_NUMBER_OF_CHANNELS];	// persistent channel data
	_TMPL_DAT 		tpl_lib[_MAX_TEMPLATES];		// schedule template library
	_S_TAB 			sch_lib[_MAX_SCHEDULES]; 		// schedule table library
	int 			sch_index;						// points to the next schedule table library available
	int 			tpl_index;						// points to the next template library available
} _SYS_DAT;

/* ipc data - memory mapped */
typedef	struct {
	int				force_update;					// force daemon to update relays
	_SYS_DAT		sys_data;						// persistant system data
	_SEN_DAT		s_dat[_NUMBER_OF_SENSORS];  	// current sensor values - set by cgi
} _IPC_DAT;

/* cmd_fsm control block */
typedef struct {
	/* fsm data */
	int				state;
	int 			p_state;
	char 			token[_MAX_TOKEN_SIZE];
	int				token_type;
	int				token_value;
	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];
	/* working data */
	int             w_channel;                      		
	int             w_minutes;
	char            w_minutes_str[4];
	int             w_hours;
	char            w_hours_str[4];
	int 			w_day;
	int				w_srestate;
	int 			w_state;
	float 			w_temp;
	float 			w_humid;
	int 			w_sensor_id;
	_TMPL_DAT 		w_template_buffer;
	_S_TAB        	w_sch;   
	/* pointers */
	_IPC_DAT 		*ipc_ptr;			// pointer to shared memory
	_SYS_DAT		*sys_ptr;			// pointer to system data in shared memory
	_S_TAB 			*ssch_ptr;			// pointer to active schedule in shared memory
	_S_TAB 			*wsch_ptr;			// pointer to working schedule
	_CHN_DAT 		*cdat_ptr; 			// pointer to channel data in shared memory
} _CMD_FSM_CB;

/* buffer for semaphores */
typedef struct sembuf SEMBUF;

/* action routine definitions */
typedef int (*CMD_ACTION_PTR)(_CMD_FSM_CB *);
typedef int (*CHAR_ACTION_PTR)(char *);

#endif
