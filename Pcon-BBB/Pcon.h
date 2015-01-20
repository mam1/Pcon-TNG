#ifndef PCON_H_
#define PCON_H_

#define _major_version 			2
#define _minor_version 			0
#define _minor_revision 		23
#define _TRACE
#define _SYSTEM_DATA_FILE		"/media/Pcon-datacd/Pcon.dat" //sd card file

/* character parser fsm */
#define _CHAR_TOKENS     		5
#define _CHAR_STATES     		4
#define _MAX_TOKEN_SIZE			128

/* command parser fsm */
#define _CMD_TOKENS     		29
#define _CMD_STATES     		3 

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	8
#define _CHANNEL_NAME_SIZE		20
#define _MAX_SCHEDULE_RECS		10
#define _INTS_PER_SCHEDULE			_MAX_SCHEDULE_RECS + 1
#define _BYTES_PER_SCHEDULE			(_INTS_PER_SCHEDULE) * 4 	//a uint32_t contains the number of active records followed by vector of uint32_ts
#define _BYTES_PER_WORKING_SET		_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* buffers */
#define _INPUT_BUFFER_SIZE		128
#define _TOKEN_BUFFER_SIZE		60
#define _PROMPT_BUFFER_SIZE		60

// #define _TIME_STRING_BUFFER     40
// #define _VCMD_BUFFER            128

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

/* shared codes, bone and prop */
#define PING         27
#define ACK           6

/* other */
#define	_BYTES_PER_INT 				4

/* fuctions */
int term(int);
void term1(void);

#endif
