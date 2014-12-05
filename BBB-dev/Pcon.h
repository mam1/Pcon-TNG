#ifndef PCON_H_
#define PCON_H_

#define _major_version 2
#define _minor_version 0
#define _minor_revision 11
#define _TRACE

// #define _FILE_SET_ID            "834"

/* configuration options */
#define _DIOB       1       //configure to drive Parallax Digital IO Board
#define _212GH      2       //configure to drive 5 AYQ212GH relays

#define _DRIVEN     _DIOB

/* character parser fsm */
#define _CHAR_TOKENS     5
#define _CHAR_STATES     4
#define _MAX_TOKEN_SIZE		30

/* command parser fsm */
#define _CMD_TOKENS     29
#define _CMD_STATES     14 

/* channel parameters */
#if _DRIVEN == _DIOB
#define _NUMBER_OF_CHANNELS 8
#else
#define _NUMBER_OF_CHANNELS 5
#endif
#define _CHANNEL_NAME_SIZE          20
// #define _CHANNEL_NAME_BUFFER_SIZE   30
// #define _CHANNEL_FILE_NAME_SIZE     30

/* sd card file parameters */
// #if _DRIVEN == _DIOB
//     #define _F_PREFIX           "dio"
// #else
//     #define _F_PREFIX           "rly"
// #endif
// #define _F_SCHEDULE_SUFIX       ".sch"
// #define _F_CHANNEL_SUFIX        ".ch"
/* schedule parameters */
// #define _DAYS_PER_WEEK          	7
#define _MAX_SCHEDULE_RECS      	10
#define _INTS_PER_SCHEDULE			_MAX_SCHEDULE_RECS + 1
#define _BYTES_PER_SCHEDULE			(_INTS_PER_SCHEDULE) * 4 	//a uint32_t contains the number of active records followed by vector of uint32_ts
#define _BYTES_PER_WORKING_SET		_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

#define	_BYTES_PER_INT 				4

// #define _SCHEDULE_BUFFER        	_BYTES_PER
// #define _SCHEDULE_FILE_NAME_SIZE    40
// #define _STATE_MASK             	B32(10000000,00000000,00000000,00000000)                 
// #define _KEY_MASK               	B32(01111111,11111111,11111111,11111111) 
// #define _BYTES_PER_CHANNEL      	(_MAX_SCHEDULE_RECS + 1) * 4
// #define _BYTES_PER_DAY          	_BYTES_PER_CHANNEL * _NUMBER_OF_CHANNELS

/* buffers */
#define _INPUT_BUFFER           128
// #define _TIME_STRING_BUFFER     40
#define _TOKEN_BUFFER           60
// #define _VCMD_BUFFER            128
// #define _PROMPT_BUFFER          400

/* key codes */
#define _ESC        27
#define _SPACE      32
#define _COLON      58
#define _SLASH      47
#define _COMMA      44
#define _BS         8
#define _DEL		127
#define _QUOTE      34
#define _CR         13
#define _NL 		10
#define _FF 		12
#define _EOF		0
#define _NO_CHAR    255
#define _DELIMITER	42	//asterisk

/* fuctions */
int term(int);
#endif
