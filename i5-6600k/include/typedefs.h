#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_


// #include <sys/sem.h>
// #include <sys/ipc.h>
// #include <sys/types.h>
// #include "Pcon.h"

// typedef struct {
// 	int    tm_sec;   // seconds [0,61]
// 	int    tm_min;   // minutes [0,59]
// 	int    tm_hour;  // hour [0,23]
// 	int    tm_mday;  // day of month [1,31]
// 	int    tm_mon;   // month of year [0,11]
// 	int    tm_year;  // years since 1900
// 	int    tm_wday;  // day of week [0,6] (Sunday = 0)
// 	int    tm_yday;  // day of year [0,365]
// 	int    tm_isdst; // daylight savings flag
// } _tm;

// /* system configuration data */
// typedef struct {
// 	int         major_version;
// 	int         minor_version;
// 	int         minor_revision;
// 	int 		channels;
// 	int 		sensors;
// 	int 		commands;
// 	int 		states;
// } _CONFIG_DAT;

// /* channel data */
// typedef struct {
// 	int 		time_on; 			// accumulated minutes of on time for channel
// 	int			on_sec;				// on cycle in seconds
// 	int 		off_sec;			// off cycle in seconds
// 	int 		mode; 				// control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
// 	int 		state;				// 0 = off, 1 = on
// 	int 		sensor_id;  		// sensor id of sensor associated with the channel
// 	int 		sensor_assigned;
// 	char 		name[_CHANNEL_NAME_SIZE];
// 	int 		hide; 				// do not dispplay channel data
// } _CHN_DAT;

typedef struct topic_node {
	char 				*token;
	struct topic_node 	*next;
} _TOPIC_NODE;

// /* sensor data */
// typedef struct {
// 	int 		sensor_id;
// 	int			group_id;
// 	int 		active;
// 	char 	 	group[_GROUP_NAME_SIZE];			// sort tag for sensor display
// 	char 		description[_DESCRIPTION_NAME_SIZE];		// description
// 	int 		channel[_NUMBER_OF_CHANNELS];  	// channel assignments
// 	int 		channel_index;					// points to the next open assingment
// 	float		temp;							// current sensor reading
// 	float		humidity;						// current sensor reading
// 	time_t 		ts; 							// time and date of the reading
// } _SEN_DAT;

// /* sensor data log file */
// typedef	struct{
// 		int 		sensor_id;
// 		float		temp;
// 		float		humidity;
// 		time_t 		ts;
// 		char 		description[_DESCRIPTION_NAME_SIZE];
// } _SEN_DAT_REC;

// /* schedule record */
// typedef struct {
// 	int			key;
// 	int 		state;
// 	float 		temp;
// 	float 		humid;
// } _S_REC;				

// /* schedule  */
// typedef struct {
// 	char 			name[_TEMPLATE_NAME_SIZE];
// 	int 			rcnt;
// 	_S_REC			rec[_MAX_SCHEDULE_RECS +1];

// } _TMPL_DAT;

// /* schedule table */
// typedef struct {
// 	_TMPL_DAT 	sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS];
// } _S_TAB;

// /* template library */
// typedef struct {
// 	_TMPL_DAT 		t_lib[_MAX_TEMPLATES];
// } _T_TAB;

// /* structure of system file record */
// typedef struct {
// 	_CONFIG_DAT 	config;							// system configuration
// 	_S_TAB  		sys_sch;						// system schedule
// 	_CHN_DAT     	c_data[_NUMBER_OF_CHANNELS];	// persistent channel data
// 	_TMPL_DAT 		tpl_lib[_MAX_TEMPLATES];		// schedule template library
// 	_S_TAB 			sch_lib[_MAX_SCHEDULES]; 		// schedule table library
// 	int 			sch_index;						// points to the next schedule table library available
// 	int 			tpl_index;						// points to the next template library available
// } _SYS_DAT;

// /* ipc data - memory mapped */
// typedef	struct {
// 	int				force_update;					// force daemon to update relays
// 	_SYS_DAT		sys_data;						// persistant system data
// 	_SEN_DAT		s_dat[_NUMBER_OF_SENSORS];  	// current sensor values - set by cgi
// } _IPC_DAT;

// /* cmd_fsm control block */
// typedef struct {
// 	/* fsm data */
// 	int				state;
// 	int 			p_state;
// 	char 			token[_MAX_TOKEN_SIZE];
// 	int				token_type;
// 	int				token_value;
// 	char 			prompt_buffer[_PROMPT_BUFFER_SIZE];
// 	/* working data */
// 	int             w_channel;                      		
// 	int             w_minutes;
// 	char            w_minutes_str[4];
// 	int             w_hours;
// 	char            w_hours_str[4];
// 	int 			w_day;
// 	int				w_srestate;
// 	int 			w_state;
// 	float 			w_temp;
// 	float 			w_humid;
// 	_SEN_DAT 		w_sen_dat;
// 	_TMPL_DAT 		w_template_buffer;
// 	_S_TAB        	w_sch;   
// 	/* pointers */
// 	_IPC_DAT 		*ipc_ptr;			// pointer to shared memory
// 	_SYS_DAT		*sys_ptr;			// pointer to system data in shared memory
// 	_S_TAB 			*ssch_ptr;			// pointer to active schedule in shared memory
// 	_S_TAB 			*wsch_ptr;			// pointer to working schedule
// 	_CHN_DAT 		*cdat_ptr; 			// pointer to channel data in shared memory
// } _CMD_FSM_CB;

// /* gpio data  */
// typedef struct {
// 	int         header;
// 	int         pin;
// 	int 		gpio;
// } _GPIO;

// /* buffer for semaphores */
// typedef struct sembuf SEMBUF;

// /* action routine definitions */
// typedef int (*CMD_ACTION_PTR)(_CMD_FSM_CB *);
// typedef int (*CHAR_ACTION_PTR)(char *);

// /* Pcon control block */
// // typedef struct $
// // {
// // 	uint8_t 		c;       							// character typed on keyboard
// // 	int				char_state;							// current state of the character processing fsm
// // 	int 			prompted = false;					// has a prompt been sent
// // 	int 			fd;									// file descriptor for ipc data file
// // 	char 			work_buffer[_INPUT_BUFFER_SIZE];	// containes the user input	
// // 	char 			*work_buffer_ptr, *end_buff, *start_buff, *move_ptr;

// // 	char 			screen_buf[_SCREEN_BUFFER_SIZE], *cursor_ptr;
// // 	char 			*input_ptr, *hptr;

// // 	char 			ring_buffer[_CMD_BUFFER_DEPTH][_INPUT_BUFFER_SIZE];	// char array[NUMBER_STRINGS][STRING_MAX_SIZE];
// // 	int 			rb_in_idx, rb_out_idx;
// // 	int 			mv;
// // 	int  			escape;				// have we just processed an escape	
// // } _PCON;

#endif
