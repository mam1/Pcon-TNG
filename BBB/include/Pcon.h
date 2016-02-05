#ifndef PCON_H_
#define PCON_H_

// #define _TRACE
#define _SYSTEM_DATA_FILE		"/home/Pcon-data/Pcon.dat" 		// system data file
#define _IPC_FILE				"/home/Pcon-data/ipc.dat"   	// memory mapped ipc file
#define _TRACE_FILE_NAME		"/home/Pcon-data/trace.dat" 	// trace file
// #define _TRACE 				// turn on trace

#include "shared.h"

/* character parser fsm */
#define _CHAR_TOKENS     		5
#define _CHAR_STATES     		4
#define _MAX_TOKEN_SIZE			128

/* command parser fsm */
#define _CMD_TOKENS     		34
#define _CMD_STATES     		22 

#define _MAX_SCHLIB_SCH			20

/* buffers */
#define _INPUT_BUFFER_SIZE		128
#define _TOKEN_BUFFER_SIZE		60
#define _PROMPT_BUFFER_SIZE		200

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

/* fuctions */
int term(int);
void term1(void);
void disp_sys(void);

#endif
