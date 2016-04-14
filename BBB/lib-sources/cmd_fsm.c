/*
* cmd_fsm.c
*
*  Created on: Nov 23, 2014
*      Author: mam1
*/

#include <unistd.h>		//sleep
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h>		//isdigit, isalnum, tolower
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"
#include "PCF8563.h"
// #include "schedule.h"
#include "ipc.h"
#include "sys_dat.h"
#include "sch.h"

/*********************** externals **************************/
extern int             	cmd_state, char_state;
extern char            	input_buffer[_INPUT_BUFFER_SIZE], *input_buffer_ptr;
extern char            	c_name[_CHANNEL_NAME_SIZE][_NUMBER_OF_CHANNELS];
extern int 		    	exit_flag;		              	//exit man loop if TRUE
extern int             	trace_flag;                   	//trace file is active
extern int             	bbb;				          	//UART1 file descriptor
extern _CMD_FSM_CB      	cmd_fsm_cb, *cmd_fsm_cb_ptr;  	//cmd_fsm control block
extern _SYS_DAT2         	sdat;                         	//system data structure
extern _IPC_DAT			ipc_dat;					  	//ipc data
extern void				*data; 							//pointer for shared memory
extern _IPC_DAT 			*ipc_ptr;
extern key_t 			skey;
extern int 				semid;
extern unsigned short 	semval;
extern struct sembuf	sb;



/* code to text conversion */
extern char *day_names_long[7];
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];
extern char *c_mode[4];

/*********************** globals **************************/

#ifdef _TRACE
char			trace_buf[128];
#endif

_tm 			tm;

/***************************************/
/*****  command  parser fsm start ******/
/***************************************/

/* command list */
char    *keyword[_CMD_TOKENS] = {
	/*  0 */    "temp",    
	/*  1 */    "*",
	/*  2 */    "humid",        
	/*  3 */    "schedule",
	/*  4 */    "?",
	/*  5 */    "clock",
	/*  6 */    "yes",     
	/*  7 */    "cancel",     
	/*  8 */    "replace",
	/*  9 */    "edit",
	/* 10 */    "delete",
	/* 11 */    "zero",
	/* 12 */    "on",
	/* 13 */    "off",
	/* 14 */    "clear",   
	/* 15 */    "status",
	/* 16 */    "time",
	/* 17 */    "sensor",
	/* 18 */    "cycle",
	/* 19 */    "startup",
	/* 20 */    "display",
	/* 21 */    "save",
	/* 22 */    "template",
	/* 23 */    "channel",
	/* 24 */    "load",
	/* 25 */    "set",
	/* 26 */    "q",
	/* 27 */    "done",
	/* 28 */    "back",
	/* 29 */    "system",
	/* 30 */    "debug",
	/* 31 */    "disp_sys_sch",
	/* 32 */    "disp_wrk_sch",
	/* 33 */    "disp_sch_lib",
	/* 34 */    "disp_tml_lib",
	/* 35 */    "INT",
	/* 36 */    "STR",
	/* 37 */    "OTHER"
};


// /* state specific command definitions for INT */
// char    *INT_def[_CMD_STATES] = {
// 	/*  0 */    "channel number",
// 	/*  1 */    "",
// 	/*  2 */    "on seconds",
// 	/*  3 */    "off seconds",
// 	/*  4 */    "template number",
// 	/*  5 */    "",
// 	/*  6 */    "hour",
// 	/*  7 */    "day number",
// 	/*  8 */    "channel number",
// 	/*  9 */    "template number",
// 	/* 10 */    "",
// 	/* 11 */    "minute",
// 	/* 12 */    "",
// 	/* 13 */    "hour",
// 	/* 14 */    "minute",
// 	/* 15 */    "second",
// 	/* 16 */    "day of the week",
// 	/* 17 */    "date",
// 	/* 18 */    "month",
// 	/* 19 */    "year",
// 	/* 20 */    "",
// 	/* 21 */    "",
// 	/* 22 */    "display",
// 	/* 23 */    "save schedule template",
// 	/* 24 */    "edit schedule template library",
// 	/* 25 */    "channel",
// 	/* 26 */    "load",
// 	/* 27 */    "unrecognized command"
// };

// /* state specific command definitions for STR */
// char    *STR_def[_CMD_STATES] = {
// 	/*  0 */    "",
// 	/*  1 */    "channel name",
// 	/*  2 */    "",
// 	/*  3 */    "",
// 	/*  4 */    "template name",
// 	/*  5 */    "",
// 	/*  6 */    "",
// 	/*  7 */    "",
// 	/*  8 */    "",
// 	/*  9 */    "",
// 	/* 10 */    "",
// 	/* 11 */    "",
// 	/* 12 */    "",
// 	/* 13 */    "",
// 	/* 14 */    "",
// 	/* 15 */    "",
// 	/* 16 */    "",
// 	/* 17 */    "",
// 	/* 18 */    "",
// 	/* 19 */    "",
// 	/* 20 */    "",
// 	/* 21 */    "",
// 	/* 22 */    "",
// 	/* 23 */    "",
// 	/* 24 */    "",
// 	/* 25 */    "",
// 	/* 26 */    "",
// 	/* 27 */    ""
// };

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] = {
	/*                       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27*/
	/*  0  temp        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 26,  0, 26, 27},
	/*  1  *           */  { 0,  1,  2,  3,  4,  5,  6,  8,  9,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/*  2  humid       */  { 0,  1,  2,  3,  4,  0,  0,  0,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 27,  0, 26, 27},
	/*  3  schedule    */  { 4,  1,  2,  3,  4,  0, 11,  0,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/*  4  ?           */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/*  5  clock       */  {13,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/*  6  yes         */  { 0,  1,  0,  3,  4,  4,  6,  0,  0,  0,  0,  6,  4, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/*  7  cancel      */  { 0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  8,  8},
	/*  8  replace     */  { 0,  1,  2,  3,  4,  5,  6,  0,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/*  9  edit        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 10  delete      */  { 0,  1,  2,  3,  4,  5,  4,  7,  8,  0,  0,  0,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4,  0, 26, 27},
	/* 11  zero        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 12  on          */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4,  0, 26, 27},
	/* 13  off         */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4,  0, 26, 27},
	/* 14  clear       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 15  status      */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 16  time        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 17  sensor      */  { 0, 21,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 18  cycle       */  { 0,  2,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 19  startup     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 20  display     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 21  save        */  { 6,  1,  2,  3,  6,  5,  4,  5,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 22  template    */  { 0,  1,  2,  3,  4,  5, 10,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 23  channel     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 24  load        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 25  set         */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 26  q           */  { 0,  1,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	/* 27  done        */  { 0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  6,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4},
	/* 28  back        */  { 0,  0,  1,  2,  0,  0,  4,  4,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0, 24, 24},
	/* 29  system      */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 30  debug       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 31 disp_sys_sch */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 32 disp_wrk_sch */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 33 disp_sch_lib */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 34 disp_tml_lib */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27},
	/* 35  INT         */  { 1,  1,  3,  0,  8,  6,  6,  8, 24,  7,  0, 12,  0, 14, 15, 16, 17, 18, 19, 20,  0,  0,  0,  0, 24,  0,  4,  4},
	/* 36  STR         */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27},
	/* 37  OTHER       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,  0,  0,  0, 21,  0,  0, 24,  0, 26, 27}};

/*cmd processor functions */
int c_0(_CMD_FSM_CB *); /* nop */
int c_1(_CMD_FSM_CB *); /* display all valid commands for the current state */
int c_2(_CMD_FSM_CB *); /* display time */
int c_3(_CMD_FSM_CB *); /* terminate program */
int c_4(_CMD_FSM_CB *); /* set working channel number*/
int c_5(_CMD_FSM_CB *); /* set channel name for working channel */
int c_6(_CMD_FSM_CB *); /* status - display channel data */
int c_7(_CMD_FSM_CB *); /* command not valid in current state */
int c_8(_CMD_FSM_CB *); /* command is not recognized */
int c_9(_CMD_FSM_CB *); /* set channel control mode to manual and turn channel on */
int c_10(_CMD_FSM_CB *); /* set channel control mode to manual and turn channel off */
int c_11(_CMD_FSM_CB *); /* set channel control mode to time */
int c_12(_CMD_FSM_CB *); /* set channel control mode to time & sensor */
int c_13(_CMD_FSM_CB *); /* set channel control mode to cycle */
int c_14(_CMD_FSM_CB *); /* display system data */
int c_15(_CMD_FSM_CB *); /* revert to previous state */
int c_16(_CMD_FSM_CB *); /* set on cycle time */
int c_17(_CMD_FSM_CB *); /* set off cycle time */
int c_18(_CMD_FSM_CB *); /* enter template maintenance mode */
int c_19(_CMD_FSM_CB *); /* set working schedule name */
int c_20(_CMD_FSM_CB *); /* set working schedule hour */
int c_21(_CMD_FSM_CB *); /* set working schedule minute */
int c_22(_CMD_FSM_CB *); /* set schedule record to on */
int c_23(_CMD_FSM_CB *); /* set set schedule record to off */
int c_24(_CMD_FSM_CB *); /* delete schedule record */
int c_25(_CMD_FSM_CB *); /* save schedule template */
int c_26(_CMD_FSM_CB *); /* delete schedule template */
int c_27(_CMD_FSM_CB *); /* update temperature in a schedule record */
int c_28(_CMD_FSM_CB *); /* update humidity in a schedule record */
int c_29(_CMD_FSM_CB *); /* set working channel */
int c_30(_CMD_FSM_CB *); /* set working day */
int c_31(_CMD_FSM_CB *); /* set working channel to all */
int c_32(_CMD_FSM_CB *); /* set working day to all */
int c_33(_CMD_FSM_CB *); /* build new schedule */
int c_34(_CMD_FSM_CB *); /* state 0 prompt */
int c_35(_CMD_FSM_CB *); /* set schedule maint prompt */
int c_36(_CMD_FSM_CB *); /* append state 0 prompt to prompt buffer */
int c_37(_CMD_FSM_CB *); /* set humidity prompt */
int c_38(_CMD_FSM_CB *); /* set temperature prompt */
int c_39(_CMD_FSM_CB *); /* replace system schedule */
int c_40(_CMD_FSM_CB *); /* set real time clock */
int c_41(_CMD_FSM_CB *); /* set real time clock hours */
int c_42(_CMD_FSM_CB *); /* set real time clock minutes */
int c_43(_CMD_FSM_CB *); /* set real time clock seconds */
int c_44(_CMD_FSM_CB *); /* set real time clock month */
int c_45(_CMD_FSM_CB *); /* set real time clock day */
int c_46(_CMD_FSM_CB *); /* set real time clock year */
int c_47(_CMD_FSM_CB *); /* set real time clock day of the week */
int c_48(_CMD_FSM_CB *); /* set PCF8563 */
int c_49(_CMD_FSM_CB *); /* set channel sensor_id */
int c_50(_CMD_FSM_CB *); /* set "set"  */

/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
	/*          STATE          0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27  */
	/*  0  temp        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_38,  c_7,  c_7,  c_7},
	/*  1  *           */  { c_7,  c_8,  c_7,  c_7,  c_7,  c_7,  c_7, c_31, c_32,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  2  humid       */  { c_7,  c_3,  c_7,  c_3,  c_3,  c_3,  c_7,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_7,  c_3,  c_3,  c_3,  c_3,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_37,  c_7,  c_7,  c_7},
	/*  3  schedule    */  {c_35,  c_8,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  4  ?           */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_0,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1},
	/*  5  clock       */  {c_40,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  6  yes         */  { c_7, c_34,  c_7,  c_7, c_34,  c_7,  c_7, c_34,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_48,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  7  cancel      */  {c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/*  8  replace     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_39,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  9  edit        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 10  delete      */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_24,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_24,  c_7,  c_7,  c_7},
	/* 11  zero        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 12  on          */  { c_7,  c_9,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_22,  c_7,  c_7,  c_7},
	/* 13  off         */  { c_7, c_10,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_23,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_23,  c_7,  c_7,  c_7},
	/* 14  clear       */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 15  status      */  { c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6},
	/* 16  time        */  { c_2, c_11,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2},
	/* 17  sensor      */  { c_7, c_12,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 18  cycle       */  { c_7, c_13,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 19  startup     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 20  display     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 21  save        */  { c_7,  c_7,  c_7,  c_7, c_50,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 22  template    */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_0,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 23  channel     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 24  load        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 25  set         */  { c_7,  c_7,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_7,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 26  q           */  { c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3},
	/* 27  done        */  {c_34, c_34, c_34, c_34, c_34, c_34, c_18, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/* 28  back        */  {c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/* 29  system      */  { c_7,  c_7, c_14, c_14,  c_7,  c_7, c_14, c_14, c_14, c_14, c_14, c_14, c_14,  c_7, c_14, c_14, c_14, c_14, c_14, c_14,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 30  debug       */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 31 disp_sys_sch */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 32 disp_wrk_sch */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 33 disp_sch_lib */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 34 disp_tml_lib */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 35  INT         */  { c_4,  c_7, c_16, c_17, c_20,  c_7, c_20, c_29, c_21,  c_7, c_33, c_21,  c_7, c_41, c_42, c_43, c_44, c_45, c_46, c_47,  c_7, c_49,  c_7,  c_7,  c_7,  c_7, c_27, c_28},
	/* 36  STR         */  { c_7,  c_5,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 37  OTHER       */  { c_8,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_8,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7}};

/*************** start fsm support functions ********************/
int is_valid_int(const char *str)
{
	if (*str == '-')     //negative numbers
		++str;
	if (!*str)           //empty string or just "-"
		return 0;
	while (*str)         //check for non-digit chars in the rest of the string
	{
		if (!isdigit(*str))
			return 0;
		else
			++str;
	}
	return -1;
}

/* return cmd type; 0-INT, 1-QUOTE, 2-unrecognized, 3-NULL, command number (0 - xx) */
int cmd_type(char *c)
{
	int     i;
	char    *p;

	/*test for an empty command */
	if ((*c == '\0') || (*c == ' '))
		return 3;
	/* test for a quoted string*/
	if (*c == _QUOTE)
		return 36;
	/* test for a integer */
	if (is_valid_int(c))
		return 35;
	/* test for a keyword */
	for (i = 0; i < _CMD_TOKENS - 3; i++)
	{
		if (strlen(c) == strlen(keyword[i])) {
			p = c;
			while (*p != '\0') {
				*p = tolower(*p);
				p++;
			};
			if (strncmp(c, keyword[i], strlen(c)) == 0)
				return i;
		}
	}
	/* unrecognized token */

	return 2;
}

/* return token type or command number */
int token_type(char *c){
	int     i;
	char    *p;

	/*test for an empty command */
	if ((*c == '\0') || (*c == ' '))
		return _TT_NULL;
	/* test for a quoted string*/
	if (*c == _QUOTE)
		return _TT_STR;
	/* test for a integer */
	if (is_valid_int(c))
		return _TT_INT;
	/* test for a keyword */
	for (i = 0; i < _CMD_TOKENS - 3; i++)
	{
		if (strlen(c) == strlen(keyword[i])) {
			p = c;
			while (*p != '\0') {
				*p = tolower(*p);
				p++;
			};
			if (strncmp(c, keyword[i], strlen(c)) == 0)
				return i;
		}
	}
	/* unrecognized token */

	return _TT_UNREC;
}

char *dequote(char *s) {
	char        *p1, *p2;
	p1 = p2 = s;
	while (*p1 != '\0') {
		if (*p1 == _QUOTE) p1++;
		else *p2++ = *p1++;
	}
	*p2 = '\0';
	return s;
}

/* reset cmd_fsm to initial state */
void cmd_fsm_reset(_CMD_FSM_CB *cb) {
	memset(&cb->prompt_buffer, '\0', sizeof(cb->prompt_buffer));
	strcpy(cb->prompt_buffer, "\n\r> ");
	cb->state = 0;
	return;
}

/* load buffer with a list of all records in a schedule */
char *sch2text(_S_CHAN *sch, char *buf) {
	int         sch_recs,  i, key, h, m;

	/*build list of schedule records for prompt */
	*buf = '\0';
	sch_recs = sch->rcnt;
	if (sch_recs == 0)
		strcat(buf, " no schedule records");
	else
		for (i = 1; i < sch_recs + 1; i++) {
			key = sch->rec[i].key;
			h = key / 60;
			m = key % 60;
			sprintf(&buf[strlen(buf)], " %2i:%02i ", h, m);
			// strcat(buf, onoff[get_s(sch[i])]);
			strcat(buf, "\n\r");
		}
	return buf;
}

/* load schedule template list into buffer  */
char *sch2text2(_S_CHAN *sch, char *buf) {
	int         sch_recs,  i, key, h, m;

	/*build list of schedule */
	*buf = '\0';
	sch_recs = sch->rcnt;

	if (sch_recs == 0)
		strcat(buf, " no schedule records");
	else
		for (i = 1; i < sch_recs + 1; i++) {

			key = sch->rec[i].key;
			h = key / 60;
			m = key % 60;
			sprintf(&buf[strlen(buf)], " %2i:%02i ", h, m);
			// strcat(buf, onoff[get_s(sch[i])]);
		}
	return buf;
}

/* append schedule template list to buffer  */
char *make_lib_list(char *buf, _CMD_FSM_CB *cb) {

	// int             i, ii;
	// int             max_name_size;
	// char            pad[_SCHEDULE_NAME_SIZE];
	// int             pad_size;
	// char            sbuf[128];

	// if (cb->sdat_ptr->schlib_index == 0) {
	// 	strcat(cb->prompt_buffer,
	// 	       "  no schedule templates defined\r\n  enter name, in quotes, to create a new template\r\n  > ");
	// 	return buf;
	// }

	// // hit = 0;
	// for (i = 0; i < cb->sdat_ptr->schlib_index + 1; i++) {
	// 	if (cb->sdat_ptr->t_data[i].name[0] != '\0') {
	// 		// hit = 1;

	// 		max_name_size = 0;
	// 		for (i = 0; i < cb->sdat_ptr->schlib_index; i++)
	// 			if(max_name_size < strlen(cb->sdat_ptr->t_data[i].name))
	// 				max_name_size = strlen(cb->sdat_ptr->t_data[i].name);

	// 		for (i = 0; i < cb->sdat_ptr->schlib_index; i++) {
	// 			pad_size = max_name_size - strlen(cb->sdat_ptr->t_data[i].name);
	// 			pad[0] = '\0';
	// 			for (ii = 0; ii < pad_size; ii++)
	// 				strcat(pad, " ");
	// 			// printf("    %i - %s%s  %s\r\n",i,cb->sdat_ptr->schlib_index,pad,sch2text2(cb->sdat_ptr->s_data[i].schedule,buf));
	// 			sprintf(&cb->prompt_buffer[strlen(cb->prompt_buffer)], "    %i - %s%s  %s\r\n", 
	// 				i, cb->sdat_ptr->t_data[i].name, pad, sch2text2(&cb->sdat_ptr->t_data[i].temp_chan_sch, sbuf));
	// 		}
	// 	}
	// }
	// // if(hit == 0)
	// //     strcat(cb->prompt_buffer,
	// //         "  no schedule templates defined\r\n  enter name, in quotes, to create a new template\r\n  > ");
	// // else
	// strcat(cb->prompt_buffer,
	//        "  enter template number to edit or name to create a new template\r\n  > ");
	return buf;
}
/* print schedule template list */
void print_tlist(_CMD_FSM_CB *cb) {
	// int             i, ii;
	// int             max_name_size;
	// char            pad[_SCHEDULE_NAME_SIZE];
	// int             pad_size;
	// char            buf[128];


	// max_name_size = 0;
	// for (i = 0; i < cb->sdat_ptr->schlib_index; i++)
	// 	if (max_name_size < strlen(cb->sdat_ptr->t_data[i].name))
	// 		max_name_size = strlen(cb->sdat_ptr->t_data[i].name);

	// for (i = 0; i < cb->sdat_ptr->schlib_index; i++) {
	// 	pad_size = max_name_size - strlen(cb->sdat_ptr->t_data[i].name);
	// 	pad[0] = '\0';
	// 	for (ii = 0; ii < pad_size; ii++)
	// 		strcat(pad, " ");
	// 	printf("    %i - %s%s  %s\r\n", 
	// 		i, cb->sdat_ptr->t_data[i].name, pad, sch2text2(&cb->sdat_ptr->t_data[i].temp_chan_sch, buf));
	// }

	return;
}

/**************** start command fsm action routines ******************/
/* do nothing */
int c_0(_CMD_FSM_CB *cb)
{
	cb->prompt_buffer[0] = '\0';
	cb->prompt_buffer[1] = ' ';
	cb->prompt_buffer[2] = '\0';
	return 0;
}
/* display all valid commands for the current state */
int c_1(_CMD_FSM_CB *cb)
{
	int         i, ii;
	int         dots;

	printf("\r\ncurrent state %i \n", cb->state);
	printf("\r\ncommands valid in any state\r\n  ESC .... reset command processor\r\n");
	printf("  time ... display time and date read from the real time clock\r\n");
	printf("  status . display channel state and data for all channels\r\n");
	printf("  ? ...... display current state and list all valid commands\r\n");
	printf("  q ...... terminate the application\r\n");
	printf("\r\ncommands valid in any state other than state 0\r\n");
	printf("  done ... terminate active function\r\n");
	printf("  back ... return to previous state\r\n");
	printf("\r\nadditional commands valid in state %i\r\n",cb->state);

	/* figure how many spacer dots are required */
	dots = 0;
	for (i = 0; i < _CMD_TOKENS; i++) {
		if ((strlen(keyword[i]) > dots)) {
			dots = strlen(keyword[i]);
		}
	}
	/* print list of valid commands */
	for (i = 0; i < _CMD_TOKENS; i++) {
		if((cmd_action[i][cb->state] == c_8) || (cmd_action[i][cb->state] == c_7) || (cmd_action[i][cb->state] == c_0) 
			|| (cmd_action[i][cb->state] == c_1) || (cmd_action[i][cb->state] == c_3) || (cmd_action[i][cb->state] == c_34)
			|| (cmd_action[i][cb->state] == c_2) || (cmd_action[i][cb->state] == c_6))
			continue;
		else {
			printf("  %s ", keyword[i]);
	        for(ii=0;ii<((dots + 2) - strlen(keyword[i]));ii++)
	            printf(".");
	        // printf(" %s",cmd_def(i, cb->state));
	        // if((i==35)  || (i==36))
	        printf(" %s",cmd_def(cb->state, i));
	        printf("\r\n");
		}
	}
	/* build prompt */
	c_34(cb);  // state 0 prompt
	return 0;
}
/* display time and date from PCF8563 */
int c_2(_CMD_FSM_CB *cb)
{
	_tm         tm;
	int         rtc;

	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);	// Open the i2c-0 bus 
	get_tm(rtc, &tm);							// read the clock 
	sleep(1);
	printf(" %02i:%02i:%02i  %s %02i/%02i/%02i\n\r",
	       tm.tm_hour, tm.tm_min, tm.tm_sec, day_names_long[tm.tm_wday], tm.tm_mon, tm.tm_mday, tm.tm_year);
	close(rtc);

	return 0;
}
/* terminate program */
int c_3(_CMD_FSM_CB *cb)
{
	term(1);
	return 0;
}
/* set working channel number */
int c_4(_CMD_FSM_CB *cb)
{
	if (cb->token_value < _NUMBER_OF_CHANNELS) {
		cb->w_channel = cb->token_value;
		cb->w_minutes = 0;
		cb->w_hours = 0;
		strcpy(cb->prompt_buffer, "enter action for channel ");
		strcat(cb->prompt_buffer, cb->token);
		// strcat(cb->prompt_buffer, "\n\r> ");

#ifdef _TRACE
	sprintf(trace_buf, "c_4 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
	sprintf(trace_buf, "c_4 set working channel to %i\n", cb->w_channel);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif

		return 0;
	}
	strcpy(cb->prompt_buffer, "channel number must be 0 to 15\r\n> ");
	return 1;
}
/* set channel name for working channel */
int c_5(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	#ifdef _TRACE
	sprintf(trace_buf, "c_5 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
	sprintf(trace_buf, "c_4 set working channel name to  %s\n", cb->token);
	#endif
	
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	strcpy(cb->sys_ptr->c_data[cb->w_channel].name, dequote(cb->token));// update ipc data

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);

	/* build prompt */
	strcpy(cb->prompt_buffer, "name set for channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, "\r\n");
	c_34(cb);   //append state 0 prompt to prompt buffer
	return 0;
}
/* status - display channel data */
int c_6(_CMD_FSM_CB *cb)
{
	int         i;
	printf("  channel  state    mode  sensor id     name\n\r");
	printf("  ----------------------------------------------------------");
	for (i = 0; i < _NUMBER_OF_CHANNELS; i++) {
		printf("\n\r   <%2i> - ", i);
		printf(" %s    %s   ", onoff[cb->sys_ptr->c_data[i].c_state], c_mode[cb->sys_ptr->c_data[i].c_mode]);
		switch(cb->sys_ptr->c_data[i].c_mode) {
		   	case 2:	// time & sensor
		      	printf("  %i     ", cb->sys_ptr->c_data[i].sensor_id);
		      	break; 
			
		   	case 3:	// cycle
		      	printf(" (%i:%i)", cb->sys_ptr->c_data[i].on_sec, cb->sys_ptr->c_data[i].off_sec);
		      	break; 
		  
		   	default : 
		   		printf("          ");
		}
		printf("%s", cb->sys_ptr->c_data[i].name);
	}
	c_34(cb);  // state 0 prompt
	return 0;
}
/* command is not valid in current state */
int c_7(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	char        hold_prompt[_PROMPT_BUFFER_SIZE];
	/* build prompt */
	strcpy(hold_prompt, cb->prompt_buffer);
	strcpy(cb->prompt_buffer, "'");
	strcat(cb->prompt_buffer, cb->token);
	strcat(cb->prompt_buffer, "' is not a valid command in state ");
	sprintf(numstr, "%d", cb->state);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, "\n\r");
	strcat(cb->prompt_buffer, hold_prompt);

	return 1;
}
/* command is not recognized */
int c_8(_CMD_FSM_CB *cb)
{
	strcpy(cb->prompt_buffer, "'");
	strcat(cb->prompt_buffer, cb->token);
	strcat(cb->prompt_buffer, "' is not a valid command\n\r> ");
	printf("%s",cb->prompt_buffer);
	strcpy(cb->prompt_buffer, "\0");
	return 1;
}
/* set channel control mode to manual and turn channel on */
int c_9(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].c_mode = 0;	// update ipc data
	cb->sys_ptr->c_data[cb->w_channel].c_state = 1;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);

	/* build prompt */
	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " turned on and mode set to manual\r\n");
	c_34(cb);   //append state 0 prompt to prompt buffer

	return 0;
}
/* set channel control mode to manual and turn channel off */
int c_10(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].c_mode = 0;	// update ipc data
	cb->sys_ptr->c_data[cb->w_channel].c_state = 0;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);

	/* build prompt */
	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " turned off and mode set to manual\r\n");
	c_34(cb);   //append state 0 prompt to prompt buffer

	return 0;
}
/* set channel control mode to time */
int c_11(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	printf("cb->w_channel <%i>\n\r", cb->w_channel);

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].c_mode = 1;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory
					
	
	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);

	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " mode set to time\r\n");
	c_34(cb);   //append state 0 prompt to prompt buffer

	return 0;
}
/* set channel control mode to time and sensor */
int c_12(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].c_mode = 2;
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	// sdat.c_data[cb->w_channel].c_mode = 2;

	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);

	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " mode set to time & sensor\r\nenter sensor id number");
	return 0;
}
/* set channel control mode to cycle */
int c_13(_CMD_FSM_CB *cb)
{
// 	char        sbuf[20];

// 	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

// 	ipc_ptr->c_dat[cb->w_channel].c_mode = 3;
// 	ipc_ptr->force_update = 1;					// force relays to be updated

// 	ipc_sem_free(semid, &sb);					// free lock on shared memory

// 	sdat.c_data[cb->w_channel].c_mode = 3;
// 	sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);

// 	strcpy(cb->prompt_buffer, "  setting cycle mode for channel ");
// 	sprintf(sbuf, "%d", cb->w_channel);
// 	strcat(cb->prompt_buffer, sbuf);
// 	strcat(cb->prompt_buffer, "\r\n  enter on seconds > ");
	return 0;
}
/* display system data */
int c_14(_CMD_FSM_CB *cb)
{
	// // char            temp_buf[128];

	// printf("\r\ncurrent state %i\r\n\n", cb->state);
	// printf("\r\n******* system data ***************************************************************\r\n\n");
	// sys_disp(cb->sdat_ptr);
	// printf("\r\n******* schedule templates ********************************************************\r\n\n");
	// print_tlist(cb);
	// printf("\r\n******* system schedule ***********************************************************\r\n");
	// disp_all_schedules(&cb->sdat_ptr->sys_sch);

	// c_34(cb);   // state 0 prompt
	return 0;
}
/* move back to previous state */
int c_15(_CMD_FSM_CB *cb)
{

	cb->state = cb->p_state;
	strcpy(cb->prompt_buffer, "\r\n> ");

	return 0;
}
/* set on cycler time */
int c_16(_CMD_FSM_CB *cb)
{
	// char            sbuf[20];  //max number of digits for a int

	// sdat.c_data[cb->w_channel].on_sec = cb->token_value;
	// sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);

	// /* build prompt */
	// strcpy(cb->prompt_buffer, "  setting cycle mode for channel ");
	// sprintf(sbuf, "%d", cb->w_channel);
	// strcat(cb->prompt_buffer, sbuf);
	// strcat(cb->prompt_buffer, " on ");
	// sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].on_sec);
	// strcat(cb->prompt_buffer, sbuf);
	// strcat(cb->prompt_buffer, " seconds\r\n  enter off seconds > ");

	return 0;
}
/* set off cycle time */
int c_17(_CMD_FSM_CB *cb)
{
	// char            sbuf[20];  //max number of digits for a int

	// sdat.c_data[cb->w_channel].off_sec = cb->token_value;
	// sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);
	// /* build prompt */
	// strcpy(cb->prompt_buffer, "  channel ");
	// sprintf(sbuf, "%d", cb->w_channel);
	// strcat(cb->prompt_buffer, sbuf);
	// strcat(cb->prompt_buffer, " mode set to cycle ");
	// sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].on_sec);
	// strcat(cb->prompt_buffer, sbuf);
	// strcat(cb->prompt_buffer, ":");
	// sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].off_sec);
	// strcat(cb->prompt_buffer, sbuf);
	// strcat(cb->prompt_buffer, " (on:off)\r\n\n> ");
	return 0;
}

/* enter template maintenance mode */
int c_18(_CMD_FSM_CB *cb)
{

	/* build prompt */
	strcpy(cb->prompt_buffer, " schedule template maintenance\r\n\ntemplate library\r\n");
	make_lib_list(cb->prompt_buffer, cb);
	return 0;
}

/* set working template name */
int c_19(_CMD_FSM_CB *cb)
{
	// strcpy((char *)cb->w_schedule_name, cb->token);
	// dequote((char *)cb->w_schedule_name);

	// /* build prompt */
	// strcpy(cb->prompt_buffer, "editing schedule template: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, "\r\n  enter time (HH,MM) > ");
	return 0;
}

/* set schedule hour */
int c_20(_CMD_FSM_CB *cb)
{
	if ((cb->token_value > 23) || (cb->token_value < 0)) {
		strcpy(cb->prompt_buffer, " hour must be 0 - 23\r\n  enter hour");
		return 1;
	}

	/* update control block */
	cb->w_hours = cb->token_value;
	strcpy(cb->w_hours_str, cb->token);

	/* build prompt */
	// strcpy(cb->prompt_buffer, "editing schedule: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, " ");
	// strcat(cb->prompt_buffer, cb->w_hours_str);
	// strcat(cb->prompt_buffer, ":\r\n");
	strcpy(cb->prompt_buffer, " enter minute > ");
	return 0;
}

/* set schedule minute */
int c_21(_CMD_FSM_CB *cb)
{
	// char            temp[_PROMPT_BUFFER_SIZE];

	/* check value */
	if ((cb->token_value > 59) || (cb->token_value < 0)) {
		strcpy(cb->prompt_buffer, " minute must be 0 - 59\r\n  enter minute");
		return 1;
	}
	/* update control block */
	cb->w_minutes = cb->token_value;
	strcpy(cb->w_minutes_str, cb->token);

	/* build prompt */
	// strcpy(cb->prompt_buffer, "editing schedule template: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, "\r\n");


	// load_temps(&cb->w_template_buffer, cb->prompt_buffer);

	strcpy(cb->prompt_buffer, " enter action for ");
	strcat(cb->prompt_buffer, cb->w_hours_str);
	strcat(cb->prompt_buffer, ":");
	strcat(cb->prompt_buffer, cb->w_minutes_str);
	// strcat(cb->prompt_buffer, " > ");
	return 0;
}

/* set schedule record to on */
int c_22(_CMD_FSM_CB *cb)
{
	int 			i;
	// int 			key;
	_S_REC 			hold;

// printf("c_22 called\r\n");

	// key = cb->w_hours * 60 + cb->w_minutes;

	/* serch for key in a schedule */
	i = find_tmpl_key(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);
	if(i != -1)
		hold = cb->w_template_buffer.rec[i];
	else{
		hold.temp = 0;
		hold.humid = 0;
	}

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, 1, hold.temp, hold.humid);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}

/* set set schedule record to off */
int c_23(_CMD_FSM_CB *cb)
{
	int 			i;
	// int 			key;
	_S_REC 			hold;

	// key = cb->w_hours * 60 + cb->w_minutes;
		/* serch for key in a schedule */
	i = find_tmpl_key(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);
	if(i != -1)
		hold = cb->w_template_buffer.rec[i];
	else{
		hold.temp = 0;
		hold.humid = 0;
	}

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, 0, hold.temp, hold.humid);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}



/* delete schedule record */
int c_24(_CMD_FSM_CB *cb)
{
	del_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}

/* save schedule template */
int c_25(_CMD_FSM_CB *cb)
{
	// int             i, index;


	// if (cb->sdat_ptr->schlib_index ==  0) {
	// 	index = 0;
	// 	cb->sdat_ptr->schlib_index += 1;
	// }
	// else
	// 	for (i = 0; i < cb->sdat_ptr->schlib_index + 1; i++) {
	// 		if (strcmp(cb->sdat_ptr->t_data[i].name, (char *)cb->w_schedule_name) == 0) {
	// 			index = i;
	// 			break;
	// 		}
	// 	}
	// if ((index != 0) && (index != i)) {
	// 	index = cb->sdat_ptr->schlib_index;
	// 	cb->sdat_ptr->schlib_index += 1;
	// }

	// strcpy(cb->sdat_ptr->t_data[index].name, (char *)cb->w_schedule_name);      //copy name
	

	// for (i = 0; i < _SCHEDULE_SIZE; i++) {
	// 	cb->sdat_ptr->t_data[index].schedule[i]  = cb->w_schedule[i];   //copy schedule
	// 	cb->w_schedule[i] = '\0';                                       //clear working shcedule
	// }
	// cb->sdat_ptr->t_data
	// add_tem_rec2(cb->sdat_ptr->t_data, cb->w_hour, cb->w_minute, cb->w_state, cb->w_temp, cb->w_humid)

	// sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);

	// /* build prompt */
	// strcpy(cb->prompt_buffer, "\r\nschedule template: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, " is saved\r\n\n");
	// strcat(cb->prompt_buffer, "schedule maintenance\r\n");
	// make_lib_list(cb->prompt_buffer, cb);

	return 0;
}

/* delete schedule template */
int c_26(_CMD_FSM_CB *cb)
{

	// // char            temp[200];
	// int                i, ii;

	// // printf("wipe out w index %i\r\n",cb->w_template_index);
	// // printf("wipe out d index %i\r\n",cb->sdat_ptr->schlib_index);

	// if (cb->w_template_index == (cb->sdat_ptr->schlib_index - 1)) {                                  //delete high entry
	// 	cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index - 1;                                //back down index
	// 	memset(cb->sdat_ptr->s_data[cb->w_template_index].name, '\0',
	// 	       sizeof(cb->sdat_ptr->s_data[cb->w_template_index].name));                               //wipe name
	// 	memset(cb->sdat_ptr->s_data[cb->w_template_index].schedule, '\0',
	// 	       sizeof(cb->sdat_ptr->s_data[cb->w_template_index].schedule));                           //wipe schedule
	// 	// cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index -1;                                 //back down index

	// }
	// else {
	// 	for (i = cb->w_template_index; i < (cb->sdat_ptr->schlib_index); i++) {
	// 		strcpy(cb->sdat_ptr->schlib_index, cb->sdat_ptr->s_data[i + 1].name);                 //copy name
	// 		for (ii = 0; ii < _SCHEDULE_SIZE; ii++) {
	// 			cb->sdat_ptr->s_data[i].schedule[ii] = cb->sdat_ptr->s_data[ii + 1].schedule[ii];   //copy schedule
	// 		}
	// 	}
	// 	cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index - 1;                                //back down index

	// }
	// cb->w_template_index = cb->w_template_index - 1;


	
	// sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);

	// /* build prompt */
	// strcpy(cb->prompt_buffer, "\r\nschedule template: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, " deleted\r\n\n");
	// strcat(cb->prompt_buffer, "schedule maintenance\r\n");
	// make_lib_list(cb->prompt_buffer, cb);

	return 0;
}

/* update temperature in a schedule record */
int c_27(_CMD_FSM_CB *cb)
{
	int 			i;
	// int 			key;
	_S_REC 			hold;


	// key = cb->w_hours * 60 + cb->w_minutes;

	/* serch for key in a schedule */
	i = find_tmpl_key(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);
	if(i != -1)
		hold = cb->w_template_buffer.rec[i];
	else{
		hold.state = 0;
		hold.humid = 0;
	}

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, hold.state, cb->token_value, hold.humid);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}

/* update humidity in a schedule record*/
int c_28(_CMD_FSM_CB *cb)
{
	int 			i;
	// int 			key;
	_S_REC 			hold;


	// key = cb->w_hours * 60 + cb->w_minutes;

	/* serch for key in a schedule */
	i = find_tmpl_key(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);
	if(i != -1)
		hold = cb->w_template_buffer.rec[i];
	else{
		hold.state = 0;
		hold.temp = 0;
	}

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, hold.state,hold.temp, cb->token_value);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}

/* set working channel */
int c_29(_CMD_FSM_CB *cb)
{

// 	// printf("editing system schedule\r\n");
// 	cb->w_channel = cb->token_value;

// 	/* build prompt */
// 	strcpy(cb->prompt_buffer, "\r\n  enter day number or *  > ");
	return 0;
}

/* set working day */
int c_30(_CMD_FSM_CB *cb)
{
// 	if ((cb->token_value > 0) && (cb->token_value < _DAYS_PER_WEEK + 1)) {
// 		cb->w_day = cb->token_value - 1;
// 		return 0;
// 	}
// 	strcpy(cb->prompt_buffer, "\r\n  day number must be 1 to 7 > ");
// 	return 1;

// 	/* build prompt */
// 	strcpy(cb->prompt_buffer, "\r\n  enter day number or * > ");

	return 0;

}

/* set working channel to all */
int c_31(_CMD_FSM_CB *cb)
{
	// // printf("editing system schedule\r\n");
	// cb->w_channel = _ALL_CHANNELS;
	// /* build prompt */
	// strcpy(cb->prompt_buffer, "\r\n  enter day number or * > ");
	return 0;
}

/* set working day to all */
int c_32(_CMD_FSM_CB *cb)
{
	// // printf("editing system schedule\r\n");
	// cb->w_day = _ALL_DAYS;

	// /* build prompt */
	// strcpy(cb->prompt_buffer, "\r\n  enter template number  > ");
	return 0;
}

/* update schedule data*/
int c_33(_CMD_FSM_CB *cb)
{
// 	int             channel, template, day;
// 	// int              i, ii, iii;

// 	template = cb->w_template_num;
// 	day = cb->w_day;
// 	channel = cb->w_channel;
// 	if (cb->w_channel == _ALL_CHANNELS)
// 		printf("  setting all channels ");
// 	else {
// 		channel = cb->w_channel;
// 		printf("  setting channel %i ", cb->w_channel);
// 	}
// 	if (cb->w_day == _ALL_DAYS)
// 		printf("all days to schedule template %i\r\n", cb->w_template_num);
// 	else {
// 		day = cb->w_day;
// 		printf("day %i to schedule template %i\r\n", cb->w_day, cb->w_template_num);
// 	}

// 	// printf("loadinf schedule for day %i channel %i with %i\r\n", cb->w_day, cb->w_channel, cb->w_template_num);
// 	// load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[cb->w_template_num].schedule, cb->w_day, cb->w_channel);   //(schedule data, template, day, channel)




// 	if ((cb->w_channel == _ALL_CHANNELS) && (cb->w_day == _ALL_DAYS)) {
// 		for (day = 0; day < _DAYS_PER_WEEK; day++)
// 			        for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++)
// 				             load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer
// 	}
// 	        else if (cb->w_day == _ALL_DAYS)
// 		        for (day = 0; day < _DAYS_PER_WEEK; day++)
// 			             load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer
// 	        else if (cb->w_channel == _ALL_CHANNELS)
// 		        for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++)
// 			             load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer
// 	        else
// 		        load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer

// 	        disp_all_schedules(&cb->sdat_ptr->sys_sch);

// 	        /* build prompt */
// 	        strcpy(cb->prompt_buffer, "\r\n  enter channel{N(0...7)|*},day{N(1...7)|*},template{N}  > ");
	        return 0;
}

/* state 0 prompt */
int c_34(_CMD_FSM_CB *cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n enter a command");
	return 0;
}

/* set schedule maint prompt */
int  c_35(_CMD_FSM_CB *cb)
{
	/* build prompt */
	cb->prompt_buffer[0] = '\0';
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n enter a command or time");
	return 0;
}

/* append state 0 prompt to prompt buffer */
int c_36(_CMD_FSM_CB *cb)
{
	/* build prompt */
	strcat(cmd_fsm_cb.prompt_buffer, "\r\n enter a command");
	return 0;
}

/* display debug data */
int c_37(_CMD_FSM_CB *cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer,"\r\n enter trigger humidity");
	return 0;
}

/* set temperature prompt */
int c_38(_CMD_FSM_CB *cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer,"\r\n enter trigger temperature");

	return 0;
}

/* replace system schedule */
int c_39(_CMD_FSM_CB *cb)
{
	// // char            temp_buf[128];

	// ipc_sem_lock(semid, &sb);																// wait for a lock on shared memory
	// memcpy(ipc_ptr->sch, cmd_fsm_cb.w_sch_ptr, sizeof(cmd_fsm_cb.w_sch));					// move working schedule from fsm controol block to shared memory
	// ipc_ptr->force_update = 1;																// force relays to be updated
	// ipc_sem_free(semid, &sb);																// free lock on shared memory
	// memcpy(cmd_fsm_cb.sdat_ptr->sch_ptr, cmd_fsm_cb.w_sch_ptr, sizeof(cmd_fsm_cb.w_sch));	// move working schedule to system schedule in fsm control block
	// sys_save(_SYSTEM_DATA_FILE,cb->sdat_ptr);
	// printf("\r\n*** system schedule replaced ***\r\n");

	// /* build prompt */
	// c_34(cb);

	return 0;
}

/* set real time clock */
int c_40(_CMD_FSM_CB *cb)
{
#ifdef _TRACE
	sprintf(trace_buf, "c_40 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter time, date and day of the week - <Hour>:<Min>:<Sec> <month>/<day>/<year> <dow>\r\n> ");
	return 0;
}

/* set real time clock hours */
int c_41(_CMD_FSM_CB *cb)
{
#ifdef _TRACE
	sprintf(trace_buf, "c_41 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
	tm.tm_hour = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter minutes\r\n> ");
	return 0;
}

/* set real time clock minutes */
int c_42(_CMD_FSM_CB *cb)
{
	tm.tm_min = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter seconds\r\n> ");
	return 0;
}

/* set real time clock seconds */
int c_43(_CMD_FSM_CB *cb)
{
	tm.tm_sec = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter month\r\n> ");
	return 0;
}

/* set real time clock month */
int c_44(_CMD_FSM_CB *cb)
{
	tm.tm_mon = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter day\r\n> ");
	return 0;
}

/* set real time clock day */
int c_45(_CMD_FSM_CB *cb)
{
	tm.tm_mday = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter year\r\n> ");
	return 0;
}

/* set real time clock year */
int c_46(_CMD_FSM_CB *cb)
{
	tm.tm_year = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter day of the week number <sun=0, mon=1, ...>\r\n> ");
	return 0;
}

/* set real time clock day of the week */
int c_47(_CMD_FSM_CB *cb)
{
	tm.tm_wday = cb->token_value;
	printf(" set real time clock to:  %02i:%02i:%02i  %s %02i/%02i/%02i\n\n\r",
	       tm.tm_hour, tm.tm_min, tm.tm_sec, day_names_long[tm.tm_wday], tm.tm_mon, tm.tm_mday, tm.tm_year);
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n<yes><cancel>\r\n> ");
	return 0;
}

/* set PCF8563  */
int c_48(_CMD_FSM_CB *cb)
{
	int         rtc;

	/* Open the i2c-0 bus */
	rtc = open_tm(I2C_BUSS, PCF8583_ADDRESS);
	/* read the clock */
	set_tm(rtc, &tm);
	close(rtc);

	c_2(cb);

	/* build prompt */
	c_34(cb);
	return 0;
}

/* set sensor id  */
int c_49(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);									// wait for a lock on shared memory
	cb->sys_ptr->c_data[cb->w_channel].sensor_id = cb->token_value;
	cb->ipc_ptr->force_update  = 1;									// force relays to be updated
	ipc_sem_free(semid, &sb);									// free lock on shared memory

	f = sys_open(_SYSTEM_DATA_FILE,cb->sys_ptr);
	sys_save(f,cb->sys_ptr);	// write data to disk
	fclose(f);
	
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "sensor id for channel ");
	sprintf(numstr, "%d set to ", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	sprintf(numstr, "%d", cb->sys_ptr->c_data[cb->w_channel].sensor_id);
	strcat(cb->prompt_buffer, numstr);
	// strcat(cb->prompt_buffer, "\r\n");
	c_36(cb);		
	return 0;
}


/* set save prompt */
int c_50(_CMD_FSM_CB *cb)
{
	
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " save template or schedule?");	
	return 0;
}

/* save template */
int c_51(_CMD_FSM_CB *cb)
{
	/* test for first record */
	if(cb->sys_ptr->tpl_index == 0){
		cb->sys_ptr->tpl_index = 1;
		strcpy(cmd_fsm_cb.prompt_buffer, " template saved");
		c_36(cb);
	}
	/* see if there is room to save template */
	if((cb->sys_ptr->tpl_index + 1 ) > _MAX_TEMPLATES){
		strcpy(cmd_fsm_cb.prompt_buffer, " no room for another template\n template not saved");
		c_36(cb);
		return 0;
	}
	cb->sys_ptr->tpl_lib[cb->sys_ptr->tpl_index] = cb->w_template_buffer;
	cb->sys_ptr->tpl_index += 1;
	
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " template saved");
	c_36(cb);	
	return 0;
}

/* save sched */
int c_52(_CMD_FSM_CB *cb)
{
	
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " save template or schedule?");	
	return 0;
}


/**************** end command fsm action routines ******************/

/* cycle state machine */
void cmd_fsm(_CMD_FSM_CB *cb)
{
	int         num, index;


	// cb->token_type = cmd_type(cb->token);

// #ifdef _TRACE
// 	sprintf(trace_buf, "cmd_fsm called: token <%s>, token value <%i>, token type <%i>, state <%i>, new token type <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state, token_type(cb->token));
// 	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
// #endif

	/* set up control block values based on token type */
	cb->token_type = token_type(cb->token);
	switch(cb->token_type){
		case _TT_INT:
			sscanf(cb->token, "%u", &num);
			cb->token_value = num;
			index = 35;
			break;
		case _TT_STR:
			cb->token_value = -1;
			index = 36;
			break;
		case _TT_UNREC:
			cb->token_value = -1;
			index = 37;
			break;
		case _TT_NULL:
			return;
			break;
		default:
			cb->token_value = cb->token_type;
			if((cb->token_value < 0) || (cb->token_value > _CMD_TOKENS )){
				printf("\n\r****************************************\n\r");
				printf("**** command number out of range *******\n\r");
				printf("************** %2i **********************\n\r",cb->token_value);
				printf("****************************************\n\r");
				printf("\r\naborting application\n\r");
				term1();
			}
			index = cb->token_value;
	}

#ifdef _TRACE
	sprintf(trace_buf, "cmd_fsm called before setting new state: index <%i>token <%s>, token value <%i>, token type <%i>, state <%i>\n", index, cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif

	if (cmd_action[index][cb->state](cb) == 0) {		// fire off a fsm action routine
		cb->p_state = cb->state;
		cb->state = cmd_new_state[index][cb->state];	// update state

#ifdef _TRACE
	sprintf(trace_buf, "cmd_fsm called after setting new state: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif	
	}         //transition to next state
	else
	{
#ifdef _TRACE	
   printf("error returned from action routine\n\r");
#endif	
		while (pop_cmd_q(cmd_fsm_cb.token)); //empty command queue
	}
	return;
}



