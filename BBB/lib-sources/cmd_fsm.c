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
#include "list_maint.h"
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
extern _CMD_FSM_CB      cmd_fsm_cb, *cmd_fsm_cb_ptr;  	//cmd_fsm control block
extern _SYS_DAT         sdat;                         	//system data structure
extern _IPC_DAT			ipc_dat;					  	//ipc data
extern void				*data; 							//pointer for shared memory
extern _IPC_DAT 		*ipc_ptr;
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
extern char *mode[4];

/*********************** globals **************************/
#if defined (_ATRACE) || defined (_FTRACE)
char			trace_buf[128];
#endif
_tm 			tm;			// buffer fir time abd date

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
	/* 31 */    "ssch",
	/* 32 */    "wsch",
	/* 33 */    "slib",
	/* 34 */    "tlib",
	/* 35 */    "INT",
	/* 36 */    "STR",
	/* 37 */    "OTHER"
};

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] = {
	/*                       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28 */
	/*  0  temp        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 26, 25, 26, 27, 28},
	/*  1  *           */  { 0,  1,  2,  3,  4,  9,  6,  8,  9,  4, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/*  2  humid       */  { 0,  1,  2,  3,  4,  0,  0,  0,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 27, 25, 26, 27, 28},
	/*  3  schedule    */  { 4,  1,  2,  3,  4,  0, 11, 23,  8,  9, 10, 11, 28, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/*  4  ?           */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/*  5  clock       */  {13,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/*  6  yes         */  { 0,  1,  0,  3,  4,  0,  6,  0,  0,  0, 10, 11,  4, 13, 14, 15, 16, 17, 18, 19,  0, 21,  0,  0, 24, 25, 26, 27, 28},
	/*  7  cancel      */  { 0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 25,  8,  8, 28},
	/*  8  replace     */  { 0,  1,  2,  3,  4,  5,  6,  0,  8,  9, 10, 11,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24, 25, 26, 27, 28},
	/*  9  edit        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 10  delete      */  { 0,  1,  2,  3, 12,  5,  4,  7,  8,  9, 10, 11,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4, 25, 26, 27, 28},
	/* 11  zero        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 12  on          */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4, 25, 26, 27, 28},
	/* 13  off         */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  6, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0,  4, 25, 26, 27, 28},
	/* 14  clear       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,  0, 22, 23, 24, 25, 26, 27, 28},
	/* 15  status      */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 16  time        */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 17  sensor      */  { 0, 21,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 18  cycle       */  { 0,  2,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 19  startup     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 20  display     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 21  save        */  { 6,  1,  2,  3,  6,  5,  4,  5,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 22  template    */  { 0,  1,  2,  3,  4,  5, 10, 22,  8,  9, 10, 11, 25, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 23  channel     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 24  load        */  { 0,  1,  2,  3,  7,  5,  6,  7,  8,  9, 10, 11,  0, 13, 14, 15, 16, 17, 18, 19, 20, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 25  set         */  { 0,  1,  2,  3,  5,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 26  q           */  { 0,  1,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 28},
	/* 27  done        */  { 0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  4,  6,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  4, 28},
	/* 28  back        */  { 0,  0,  1,  2,  0,  4,  4,  4,  4,  5,  6,  6,  4,  0, 13, 14, 15, 16, 17, 18, 19,  1,  7,  7,  8, 12, 24, 24, 12},
	/* 29  system      */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 30  debug       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 31  ssch        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 32  wsch        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 33  slib        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 34  tlib        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28},
	/* 35  INT         */  { 1,  1,  3,  0,  8,  9,  6, 21, 24,  4, 10, 11,  0, 14, 15, 16, 17, 18, 19, 20,  0,  0,  4,  4, 24,  4,  4,  4, 28},
	/* 36  STR         */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  9,  4,  4,  0, 13,  0,  0,  0,  0,  0,  0,  0, 21,  0,  0, 24, 25, 26, 27, 28},
	/* 37  OTHER       */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,  0,  0,  0, 21,  0,  0, 24, 25, 26, 27, 28}
};

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
int c_14(_CMD_FSM_CB *); /* delete entery in template library */ 
int c_15(_CMD_FSM_CB *); /* revert to previous state */
int c_16(_CMD_FSM_CB *); /* set on cycle time */
int c_17(_CMD_FSM_CB *); /* set off cycle time */
int c_18(_CMD_FSM_CB *); /* clear sensor assignment for a channel */ 
int c_19(_CMD_FSM_CB *); /*  */ //******************************************************************************
int c_20(_CMD_FSM_CB *); /* set working schedule hour */
int c_21(_CMD_FSM_CB *); /* set working schedule minute */
int c_22(_CMD_FSM_CB *); /* set schedule record to on */
int c_23(_CMD_FSM_CB *); /* set set schedule record to off */
int c_24(_CMD_FSM_CB *); /* delete schedule record */
int c_25(_CMD_FSM_CB *); /*  */
int c_26(_CMD_FSM_CB *); /*  */	//*******************************************************************************
int c_27(_CMD_FSM_CB *); /* update temperature in a schedule record */
int c_28(_CMD_FSM_CB *); /* update humidity in a schedule record */
int c_29(_CMD_FSM_CB *); /* set working channel */
int c_30(_CMD_FSM_CB *); /* set working day */
int c_31(_CMD_FSM_CB *); /* set working channel to all */
int c_32(_CMD_FSM_CB *); /* set working day to all */
int c_33(_CMD_FSM_CB *); /* clear schedule buffer */
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
int c_50(_CMD_FSM_CB *); /* set template/schedule prompt */
int c_51(_CMD_FSM_CB *); /* save schedule  */
int c_52(_CMD_FSM_CB *); /* save schedule table*/
int c_53(_CMD_FSM_CB *); /* set template description prompt  */
int c_54(_CMD_FSM_CB *); /* set schedule description prompt */
int c_55(_CMD_FSM_CB *); /* display template lib */
int c_56(_CMD_FSM_CB *); /* display schedule lib */
int c_57(_CMD_FSM_CB *); /* display system schedule */
int c_58(_CMD_FSM_CB *); /* display working schedule */
int c_59(_CMD_FSM_CB *); /* set set day prompt */
int c_60(_CMD_FSM_CB *); /* set set channel prompt */
int c_61(_CMD_FSM_CB *); /* load template  */
int c_62(_CMD_FSM_CB *); /* load schedule table*/
int c_63(_CMD_FSM_CB *); /* set template number prompt  */
int c_64(_CMD_FSM_CB *); /* set schedule number prompt */
int c_65(_CMD_FSM_CB *); /* display current sensor values*/


/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
	/*          STATE          0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27    28  */
	/*  0  temp        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_38,  c_7,  c_7,  c_7,  c_7},
	/*  1  *           */  { c_7,  c_8,  c_7,  c_7,  c_7, c_32, c_31,  c_7,  c_7, c_31,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  2  humid       */  { c_7,  c_3,  c_7,  c_3,  c_3,  c_3,  c_7,  c_7,  c_7,  c_3,  c_7,  c_3,  c_3,  c_7,  c_3,  c_3,  c_3,  c_3,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_37,  c_7,  c_7,  c_7,  c_7},
	/*  3  schedule    */  {c_35,  c_8,  c_7,  c_7,  c_7,  c_7, c_52,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  4  ?           */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_0,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_7},
	/*  5  clock       */  {c_40,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  6  yes         */  { c_7, c_34,  c_7,  c_7, c_34,  c_7,  c_7, c_34,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_48,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  7  cancel      */  {c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/*  8  replace     */  { c_7,  c_7,  c_7,  c_7, c_39,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/*  9  edit        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 10  delete      */  { c_7,  c_7,  c_7,  c_7, c_50,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_24,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_24,  c_7,  c_7,  c_7,  c_7},
	/* 11  zero        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 12  on          */  { c_7,  c_9,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_22,  c_7,  c_7,  c_7,  c_7},
	/* 13  off         */  { c_7, c_10,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_23,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_23,  c_7,  c_7,  c_7,  c_7},
	/* 14  clear       */  { c_7,  c_7,  c_7,  c_7, c_33,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_18,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 15  status      */  { c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_6,  c_7},
	/* 16  time        */  { c_2, c_11,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_2,  c_7},
	/* 17  sensor      */  {c_65, c_12,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 18  cycle       */  { c_7, c_13,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 19  startup     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 20  display     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 21  save        */  { c_7,  c_7,  c_7,  c_7, c_50,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 22  template    */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_53, c_63,  c_7,  c_7,  c_7,  c_7, c_63,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 23  channel     */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 24  load        */  { c_7,  c_7,  c_7,  c_7, c_50,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 25  set         */  { c_7,  c_7,  c_1,  c_1, c_59,  c_1,  c_1,  c_1,  c_7,  c_1,  c_1,  c_1,  c_1,  c_7,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 26  q           */  { c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_7},
	/* 27  done        */  {c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/* 28  back        */  {c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34, c_34},
	/* 29  system      */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 30  debug       */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 31  ssch        */  {c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57, c_57},
	/* 32  wsch        */  {c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58, c_58},
	/* 33  slib        */  { c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 34  tlib        */  {c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55, c_55},
	/* 35  INT         */  { c_4,  c_7, c_16, c_17, c_20, c_30, c_20,  c_7, c_21, c_29,  c_7, c_21,  c_7, c_41, c_42, c_43, c_44, c_45, c_46, c_47,  c_7, c_49, c_61,  c_7,  c_7, c_14, c_27, c_28,  c_7},
	/* 36  STR         */  { c_7,  c_5,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7, c_51,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7},
	/* 37  OTHER       */  { c_8,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_8,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7,  c_7}
};

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

// /* return cmd type; 0-INT, 1-QUOTE, 2-unrecognized, 3-NULL, command number (0 - xx) */
// int cmd_type(char *c)
// {
// 	int     i;
// 	char    *p;

// 	/*test for an empty command */
// 	if ((*c == '\0') || (*c == ' '))
// 		return 3;
// 	/* test for a quoted string*/
// 	if (*c == _QUOTE)
// 		return 36;
// 	/* test for a integer */
// 	if (is_valid_int(c))
// 		return 35;
// 	/* test for a keyword */
// 	for (i = 0; i < _CMD_TOKENS - 3; i++)
// 	{
// 		if (strlen(c) == strlen(keyword[i])) {
// 			p = c;
// 			while (*p != '\0') {
// 				*p = tolower(*p);
// 				p++;
// 			};
// 			if (strncmp(c, keyword[i], strlen(c)) == 0)
// 				return i;
// 		}
// 	}
// 	/* unrecognized token */

// 	return 2;
// }

/* return token type or command number */
int token_type(char *c) {
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
// char *sch2text(_S_CHAN *sch, char *buf) {
// 	// int         sch_recs,  i, key, h, m;

// 	// /*build list of schedule records for prompt */
// 	// *buf = '\0';
// 	// sch_recs = sch->rcnt;
// 	// if (sch_recs == 0)
// 	// 	strcat(buf, " no schedule records");
// 	// else
// 	// 	for (i = 1; i < sch_recs + 1; i++) {
// 	// 		key = sch->rec[i].key;
// 	// 		h = key / 60;
// 	// 		m = key % 60;
// 	// 		sprintf(&buf[strlen(buf)], " %2i:%02i ", h, m);
// 	// 		// strcat(buf, onoff[get_s(sch[i])]);
// 	// 		strcat(buf, "\n\r");
// 	// 	}
// 	// return buf;
// }

/* load schedule template list into buffer  */
// char *sch2text2(_S_CHAN *sch, char *buf) {
// 	// int         sch_recs,  i, key, h, m;

// 	// /*build list of schedule */
// 	// *buf = '\0';
// 	// sch_recs = sch->rcnt;

// 	// if (sch_recs == 0)
// 	// 	strcat(buf, " no schedule records");
// 	// else
// 	// 	for (i = 1; i < sch_recs + 1; i++) {

// 	// 		key = sch->rec[i].key;
// 	// 		h = key / 60;
// 	// 		m = key % 60;
// 	// 		sprintf(&buf[strlen(buf)], " %2i:%02i ", h, m);
// 	// 		// strcat(buf, onoff[get_s(sch[i])]);
// 	// 	}
// 	// return buf;
// }

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

void build_prompt(_CMD_FSM_CB * cb){
	// printf("*********** build_prompt called\r\n");
	// printf("*********** switching on cb_state = %i\r\n", cb->state);
	// printf("********** record count before call to load_temps %i\r\n", cb->w_template_buffer.rcnt);
	switch(cb->state){
		case 4:
			strcpy(cb->prompt_buffer, "\r\n editing schedule buffer\n\r");
			load_temps(&cb->w_template_buffer, cb->prompt_buffer);
			strcat(cb->prompt_buffer, "\r\n enter a command or time");
			break;

	}

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
	printf("  q ...... terminate the application\r\n");
	printf("  ? ...... display current state and list all valid commands\r\n");
	printf("  time ... display time and date read from the real time clock\r\n");
	printf("  status . display channel state and data for all channels\r\n");
	printf("  wsch ... display working schedule table\r\n");
	printf("  ssch ... display active schedule table\r\n");
	printf("  tlib ... display template library\r\n");
	printf("  slib ... display schedule library\r\n");


	printf("\r\ncommands valid in any state other than state 0\r\n");
	printf("  done ... terminate active function\r\n");
	printf("  back ... return to previous state\r\n");
	printf("\r\nadditional commands valid in state %i\r\n", cb->state);

	/* figure how many spacer dots are required */
	dots = 0;
	for (i = 0; i < _CMD_TOKENS; i++) {
		if ((strlen(keyword[i]) > dots)) {
			dots = strlen(keyword[i]);
		}
	}
	/* print list of valid commands */
	for (i = 0; i < _CMD_TOKENS; i++) {
		if ((cmd_action[i][cb->state] == c_8) || (cmd_action[i][cb->state] == c_7) || (cmd_action[i][cb->state] == c_0)
		        || (cmd_action[i][cb->state] == c_1) || (cmd_action[i][cb->state] == c_3) || (cmd_action[i][cb->state] == c_34)
		        || (cmd_action[i][cb->state] == c_2) || (cmd_action[i][cb->state] == c_6) || (cmd_action[i][cb->state] == c_55)
		        || (cmd_action[i][cb->state] == c_57) || (cmd_action[i][cb->state] == c_58))
			continue;
		else {
			printf("  %s ", keyword[i]);
			for (ii = 0; ii < ((dots + 2) - strlen(keyword[i])); ii++)
				printf(".");
			// printf(" %s",cmd_def(i, cb->state));
			// if((i==35)  || (i==36))
			printf(" %s", cmd_def(cb->state, i));
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
	strcpy(cb->prompt_buffer, "");
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

#if defined (_ATRACE) || defined (_FTRACE)
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

#if defined (_ATRACE) || defined (_FTRACE)
	sprintf(trace_buf, "c_5 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
	sprintf(trace_buf, "c_4 set working channel name to  %s\n", cb->token);
#endif

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	strcpy(cb->sys_ptr->c_data[cb->w_channel].name, dequote(cb->token));// update ipc data

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
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
	printf("\n  channel  state   mode  sensor    name\n\r");
	printf("  ----------------------------------------------------------");
	for (i = 0; i < _NUMBER_OF_CHANNELS; i++) {
		printf("\n\r%6i", i);
		printf("%9s%9s", onoff[cb->sys_ptr->c_data[i].state], mode[cb->sys_ptr->c_data[i].mode]);
	// printf(" %i    %i   ", cb->sys_ptr->c_data[i].state, cb->sys_ptr->c_data[i].mode);
		switch (cb->sys_ptr->c_data[i].mode) {
		case 2:	// time & sensor
			if(cb->sys_ptr->c_data[i].sensor_assigned == 1)
				printf("%5i", cb->sys_ptr->c_data[i].sensor_id);
			break;

		case 3:	// cycle
			printf(" (%i:%i)", cb->sys_ptr->c_data[i].on_sec, cb->sys_ptr->c_data[i].off_sec);
			break;

		default :
			printf("          ");
		}
		printf("%s", cb->sys_ptr->c_data[i].name);
	}
	printf("\n\n\r");
	strcpy(cb->prompt_buffer, "");
	// c_34(cb);  // state 0 prompt
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
	printf("%s", cb->prompt_buffer);
	strcpy(cb->prompt_buffer, "\0");
	return 1;
}
/* set channel control mode to manual and turn channel on */
int c_9(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].mode = 0;	// update ipc data
	cb->sys_ptr->c_data[cb->w_channel].state = 1;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
	fclose(f);

	/* build prompt */
	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " turned on and mode set to manual\r\n");
	c_36(cb);   //append state 0 prompt to prompt buffer

	return 0;
}
/* set channel control mode to manual and turn channel off */
int c_10(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

	cb->sys_ptr->c_data[cb->w_channel].mode = 0;	// update ipc data
	cb->sys_ptr->c_data[cb->w_channel].state = 0;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
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

	cb->sys_ptr->c_data[cb->w_channel].mode = 1;	// update ipc data
	cb->sys_ptr->c_data[cb->w_channel].state = 0;	// update ipc data
	cb->ipc_ptr->force_update = 1;					// force relays to be updated

	ipc_sem_free(semid, &sb);					// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
	fclose(f);

	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " mode set to time\r\n");
	c_36(cb);   //append state 0 prompt to prompt buffer

	return 0;
}
/* set channel control mode to time and sensor */
int c_12(_CMD_FSM_CB *cb)
{
	char        numstr[2];
	FILE 		*f;

	ipc_sem_lock(semid, &sb);						// wait for a lock on shared memory
	cb->sys_ptr->c_data[cb->w_channel].mode = 2;
	cb->sys_ptr->c_data[cb->w_channel].state = 0;	
	cb->ipc_ptr->force_update = 1;					// force relays to be updated
	ipc_sem_free(semid, &sb);						// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
	fclose(f);

	strcpy(cb->prompt_buffer, "channel ");
	sprintf(numstr, "%d", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, " mode set to sensor\r\nenter sensor id");
	return 0;
}
/* set channel control mode to cycle */
int c_13(_CMD_FSM_CB *cb)
{
// 	char        sbuf[20];

// 	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory

// 	ipc_ptr->c_dat[cb->w_channel].mode = 3;
// 	ipc_ptr->force_update = 1;					// force relays to be updated

// 	ipc_sem_free(semid, &sb);					// free lock on shared memory

// 	sdat.c_data[cb->w_channel].mode = 3;
// 	sys_save(_SYSTEM_FILE_NAME,cb->sdat_ptr);

// 	strcpy(cb->prompt_buffer, "  setting cycle mode for channel ");
// 	sprintf(sbuf, "%d", cb->w_channel);
// 	strcat(cb->prompt_buffer, sbuf);
// 	strcat(cb->prompt_buffer, "\r\n  enter on seconds > ");
	return 0;
}
/* delete entry in the template library*/
int c_14(_CMD_FSM_CB *cb)
{
	int 				i;
	FILE 				*f;

	if((cb->token_value < 0) || (cb->token_value > cb->sys_ptr->tpl_index - 1)){
		printf(" schedule number must be 0 - %i\n", cb->sys_ptr->tpl_index - 1);
		return 1;
	}

	for(i=cb->token_value;i<cb->sys_ptr->tpl_index - 1;i++)
		cb->sys_ptr->tpl_lib[i] = cb->sys_ptr->tpl_lib[i+1];
	cb->sys_ptr->tpl_index -= 1;

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);							// write data to disk
	fclose(f);
	printf("template deleted \n\r");
	c_35(cb);

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
	// sys_save(_SYSTEM_FILE_NAME,cb->sdat_ptr);

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
	// sys_save(_SYSTEM_FILE_NAME,cb->sdat_ptr);
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

/*  clear sensor assignment to a channel*/
int c_18(_CMD_FSM_CB *cb)
{
	int 		i;

	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	cb->sys_ptr->c_data[cb->w_channel].sensor_assigned = 0;		// set false
	for(i=0;i<_NUMBER_OF_SENSORS;i++)
		del_elm(cb->ipc_ptr->s_dat[i].channel, 
		&(cb->ipc_ptr->s_dat[i].channel_index), 
		cb->w_channel, 
		sizeof(cb->ipc_ptr->s_dat[i].channel));
	cb->ipc_ptr->force_update = 1;					// force relays to be updated
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* build prompt */
	// strcpy(cb->prompt_buffer, " ");

	return 0;
}

/*  */
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
	_S_REC 			hold;

	// printf("********** c_22 called, record count %i\r\n",cb->w_template_buffer.rcnt);
	/* serch for key in a schedule */
	i = find_tmpl_key(&cb->w_template_buffer, cb->w_hours, cb->w_minutes);
	if (i != -1)
		hold = cb->w_template_buffer.rec[i];
	else {
		hold.temp = 0;
		hold.humid = 0;
	}

	printf(" ** c_22 after search, record count %i\r\n",cb->w_template_buffer.rcnt);

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, 1, hold.temp, hold.humid);

	printf(" ** c_22 after add_tmpl_rec, record count %i\r\n",cb->w_template_buffer.rcnt);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	printf(" ** c_22 before call to load_temps, record count %i, prompt_buffer <%s>\r\n",cb->w_template_buffer.rcnt, cb->prompt_buffer);
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);


printf(" ** c_22 after call to load_temps, record count i, prompt_buffer <s>\r\n");
printf(" ** c_22 after call to load_temps, record count %i, prompt_buffer <%s>\r\n",99, "xxxxx");

	printf(" ** c_22 after call to load_temps, record count %i, prompt_buffer <%s>\r\n",cb->w_template_buffer.rcnt, cb->prompt_buffer);
	
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");

	printf(" ** c_22 returning, record count %i\r\n",cb->w_template_buffer.rcnt);
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
	if (i != -1)
		hold = cb->w_template_buffer.rec[i];
	else {
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

/*  */
int c_25(_CMD_FSM_CB *cb)
{


	// /* build prompt */
	// strcpy(cb->prompt_buffer, "\r\nschedule template: ");
	// strcat(cb->prompt_buffer, (char *)cb->w_schedule_name);
	// strcat(cb->prompt_buffer, " is saved\r\n\n");
	// strcat(cb->prompt_buffer, "schedule maintenance\r\n");
	// make_lib_list(cb->prompt_buffer, cb);

	return 0;
}

/*  */
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



	// sys_save(_SYSTEM_FILE_NAME,cb->sdat_ptr);

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
	if (i != -1)
		hold = cb->w_template_buffer.rec[i];
	else {
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
	if (i != -1)
		hold = cb->w_template_buffer.rec[i];
	else {
		hold.state = 0;
		hold.temp = 0;
	}

	/* add new schedule record */
	add_tmpl_rec(&cb->w_template_buffer, cb->w_hours, cb->w_minutes, hold.state, hold.temp, cb->token_value);

	/*build prompt */
	strcpy(cb->prompt_buffer, "\0");
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter command or time");
	return 0;
}

/* set working channel */
int c_29(_CMD_FSM_CB *cb) {
	char 				numstr[2];
	int 				i;

	cb->w_channel = cb->token_value;
	if (cb->w_day == _ALL_DAYS){
		for (i = 0; i < _DAYS_PER_WEEK; i++) 
			cb->wsch_ptr->sch[i][cb->w_channel] = cb->w_template_buffer;
		printf(" schedule loaded into working schedule table (all days, channel %i)\n\r",cb->w_channel );
	}
	else {
		cb->wsch_ptr->sch[cb->w_day][cb->w_channel] = cb->w_template_buffer;
		printf(" schedule loaded into working schedule table (day %i, channel %i)\n\r",cb->w_day,cb->w_channel );
	}

	strcat(cb->prompt_buffer, " channel");
	sprintf(numstr, "%2d)", cb->w_channel);
	strcat(cb->prompt_buffer, numstr);
	strcat(cb->prompt_buffer, "\r\n\n enter command");
	return 0;

}

/* set working day */
int c_30(_CMD_FSM_CB * cb)
{
	char 				numstr[2];

	if ((cb->token_value > 0) && (cb->token_value < _DAYS_PER_WEEK + 1)) {
		cb->w_day = cb->token_value - 1;
		/* build prompt */
		strcpy(cb->prompt_buffer, "day set to ");
		sprintf(numstr, "%d", cb->w_day);
		strcat(cb->prompt_buffer, numstr);
		strcat(cb->prompt_buffer, " enter channel number or * > ");

		return 0;
	}
	strcpy(cb->prompt_buffer, " day number must be 1 to 7 > ");
	return 1;

}

/* set working channel to all */
int c_31(_CMD_FSM_CB * cb)
{
	int 		c,d;
	// printf("editing system schedule\r\n");
	cb->w_channel = _ALL_CHANNELS;
	if(cb->w_day == _ALL_DAYS)
		for(d=0;d<_DAYS_PER_WEEK;d++)
			for(c=0;c<_NUMBER_OF_CHANNELS;c++)
				cb->w_sch.sch[d][c] = cb->w_template_buffer;
	else
		for(c=0;c<_NUMBER_OF_CHANNELS;c++)
			cb->w_sch.sch[cb->w_day][c] = cb->w_template_buffer;

	/* build prompt */
	strcpy(cb->prompt_buffer, " working schedule updated");
	return 0;
}

/* set working day to all */
int c_32(_CMD_FSM_CB * cb)
{
	// printf("editing system schedule\r\n");
	cb->w_day = _ALL_DAYS;

	/* build prompt */
	strcpy(cb->prompt_buffer, " day set to all, enter channel number or * for all channels");
	return 0;
}

/* clear schedule buffer */
int c_33(_CMD_FSM_CB * cb)
{
	cb->w_template_buffer.rcnt = 0;
	strcpy(cmd_fsm_cb.prompt_buffer, " schedule buffer cleared\r\n\n editing schedule buffer, enter command or time ");
	// c_34(cb);
	return 0;
}

/* state 0 prompt */
int c_34(_CMD_FSM_CB * cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n enter a command");
	return 0;
}

/* set schedule maint prompt */
int  c_35(_CMD_FSM_CB * cb)
{
	/* build prompt */
	// cb->prompt_buffer[0] = '\0';
	load_temps(&cb->w_template_buffer, cb->prompt_buffer);
	strcat(cb->prompt_buffer, "\r\n editing schedule buffer, enter a command or time");
	return 0;
}

/* append state 0 prompt to prompt buffer */
int c_36(_CMD_FSM_CB * cb)
{
	/* build prompt */
	strcat(cmd_fsm_cb.prompt_buffer, "\r\n enter a command");
	return 0;
}

/* display debug data */
int c_37(_CMD_FSM_CB * cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n enter trigger humidity");
	return 0;
}

/* set temperature prompt */
int c_38(_CMD_FSM_CB * cb)
{
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n enter trigger temperature");

	return 0;
}

/* replace system schedule */
int c_39(_CMD_FSM_CB * cb)
{
	FILE 				*f;

	ipc_sem_lock(semid, &sb);							// wait for a lock on shared memory
	*cb->ssch_ptr = *cb->wsch_ptr;						// move working schedule from fsm controol block to shared memory
	ipc_ptr->force_update = 1;							// force relays to be updated
	ipc_sem_free(semid, &sb);							// free lock on shared memory



	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);							// write data to disk
	fclose(f);

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " system schedule replaced\n\r\n");
	c_36(cb);

	return 0;
}

/* set real time clock */
int c_40(_CMD_FSM_CB * cb)
{
#if defined (_ATRACE) || defined (_FTRACE)
	sprintf(trace_buf, "c_40 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter time, date and day of the week - <Hour>:<Min>:<Sec> <month>/<day>/<year> <dow>\r\n> ");
	return 0;
}

/* set real time clock hours */
int c_41(_CMD_FSM_CB * cb)
{
#if defined (_ATRACE) || defined (_FTRACE)
	sprintf(trace_buf, "c_41 called: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
	tm.tm_hour = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter minutes\r\n> ");
	return 0;
}

/* set real time clock minutes */
int c_42(_CMD_FSM_CB * cb)
{
	tm.tm_min = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter seconds\r\n> ");
	return 0;
}

/* set real time clock seconds */
int c_43(_CMD_FSM_CB * cb)
{
	tm.tm_sec = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter month\r\n> ");
	return 0;
}

/* set real time clock month */
int c_44(_CMD_FSM_CB * cb)
{
	tm.tm_mon = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter day\r\n> ");
	return 0;
}

/* set real time clock day */
int c_45(_CMD_FSM_CB * cb)
{
	tm.tm_mday = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter year\r\n> ");
	return 0;
}

/* set real time clock year */
int c_46(_CMD_FSM_CB * cb)
{
	tm.tm_year = cb->token_value;
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\nenter day of the week number <sun=0, mon=1, ...>\r\n> ");
	return 0;
}

/* set real time clock day of the week */
int c_47(_CMD_FSM_CB * cb)
{
	tm.tm_wday = cb->token_value;
	printf(" set real time clock to:  %02i:%02i:%02i  %s %02i/%02i/%02i\n\n\r",
	       tm.tm_hour, tm.tm_min, tm.tm_sec, day_names_long[tm.tm_wday], tm.tm_mon, tm.tm_mday, tm.tm_year);
	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "\r\n<yes><cancel>\r\n> ");
	return 0;
}

/* set PCF8563  */
int c_48(_CMD_FSM_CB * cb)
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
int c_49(_CMD_FSM_CB * cb)
{
	char        numstr[15];
	FILE 		*f;
	int  		i;

	ipc_sem_lock(semid, &sb);									// wait for a lock on shared memory
	for(i=0;i<_NUMBER_OF_SENSORS;i++)
	del_elm(cb->ipc_ptr->s_dat[i].channel, 
		&(cb->ipc_ptr->s_dat[i].channel_index), 
		cb->w_channel, 
		sizeof(cb->ipc_ptr->s_dat[i].channel));

	cb->sys_ptr->c_data[cb->w_channel].sensor_id = cb->token_value;
	cb->sys_ptr->c_data[cb->w_channel].sensor_assigned = 1;		// set true
	add_elm(cb->ipc_ptr->s_dat[cb->token_value].channel, 
		&(cb->ipc_ptr->s_dat[cb->token_value].channel_index), 
		cb->w_channel, 
		sizeof(cb->ipc_ptr->s_dat[cb->token_value].channel));

	cb->ipc_ptr->force_update  = 1;								// force relays to be updated
	ipc_sem_free(semid, &sb);									// free lock on shared memory

	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
	fclose(f);
 printf("starting to build prompt\r\n");
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
int c_50(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " template or schedule?");
	return 0;
}

/* save schedule */
int c_51(_CMD_FSM_CB * cb)
{
	FILE 			*f;

	/* see if there is room to save template */
	// printf("index %i\n\r", cb->sys_ptr->tpl_index);
	if ((cb->sys_ptr->tpl_index + 1 ) > _MAX_TEMPLATES) {
		strcpy(cmd_fsm_cb.prompt_buffer, " no room for another template\n template not saved");
		c_36(cb);
		return 0;
	}

	cb->sys_ptr->tpl_lib[cb->sys_ptr->tpl_index] = cb->w_template_buffer;
	strcpy(cb->sys_ptr->tpl_lib[cb->sys_ptr->tpl_index].name, dequote(cb->token));
	cb->sys_ptr->tpl_index += 1;
	f = sys_open(_SYSTEM_FILE_NAME, cb->sys_ptr);
	sys_save(f, cb->sys_ptr);	// write data to disk
	fclose(f);

	/* build prompt */
	printf("\n\r template saved\n\r");
	c_33(cb);
	return 0;
}

/* save schedule table */
int c_52(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " save template or schedule?");
	return 0;
}

/* set template name prompt */
int c_53(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter template description");
	return 0;
}

/* set schedule name prompt */
int c_54(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter schedule name");
	return 0;
}

/* display template library */
int c_55(_CMD_FSM_CB * cb)
{
	int 			i;
	if (cb->sys_ptr->tpl_index == 0)
		printf(" no saved templates\r\n");
	else
		for (i = 0; i < cb->sys_ptr->tpl_index; i++) {
			printf(" <%i> ", i);
			list_template(&cb->sys_ptr->tpl_lib[i]);
			printf("\n\r");
		}

	/* build prompt */
	// strcpy(cb->prompt_buffer, "");
	return 0;
}

/* display schedule library */
int c_56(_CMD_FSM_CB * cb)
{
	int 			i;
	if (cb->sys_ptr->tpl_index == 0)
		printf(" no saved templates\r\n");
	else
		for (i = 0; i < cb->sys_ptr->tpl_index; i++) {
			printf(" <%i> ", i);
			// list_template(&cb->sys_ptr->tpl_lib[i]);
			printf("\n\r");
		}

	/* build prompt */
	c_34(cb);
	return 0;
}

/* display system schedule */
int c_57(_CMD_FSM_CB * cb)
{

	sch_print(cb, cb->ssch_ptr); // print a formated dump od schedules for each channel and day

	/* build prompt */
	c_34(cb);
	return 0;
}

/* display working schedule */
int c_58(_CMD_FSM_CB * cb)
{

	sch_print(cb, cb->wsch_ptr);  // print a formated dump od schedules for each channel and day

	/* build prompt */
	c_34(cb);
	return 0;
}

/* set set day prompt */
int c_59(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter day number or * for all days");
	return 0;
}

/* set set channel prompt */
int c_60(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter channel number or * for all channels");
	return 0;
}

/* load template */
int c_61(_CMD_FSM_CB * cb)
{
	if ((cb->token_value < 0) || (cb->token_value > (cb->sys_ptr->tpl_index - 1))) {
		printf(" number must be between 0 and %i\r\n", cb->sys_ptr->tpl_index - 1);
	}
	cb->w_template_buffer = cb->sys_ptr->tpl_lib[cb->token_value];

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " schedule loaded from library\n\r ");
	c_35(cb);
	return 0;
}

/* load schedule table*/
int c_62(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter channel number or * for all channels");
	return 0;
}

/* set template number prompt */
int c_63(_CMD_FSM_CB * cb)
{


	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter template number");
	return 0;
}

/* set schedule table number prompt */
int c_64(_CMD_FSM_CB * cb)
{

	/* build prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter enter table number");
	return 0;
}

/* display curent sensor values */
int c_65(_CMD_FSM_CB * cb)
{
	int 			sensor,i;

	printf("\n  sensor temp  humid channels\r\n");
	printf("  ------------------------------------------------\r\n");
	for(sensor=0;sensor<_NUMBER_OF_SENSORS;sensor++){
		printf("%6i%7i%6i", sensor, cb->ipc_ptr->s_dat[sensor].temp, cb->ipc_ptr->s_dat[sensor].humidity);
		for(i=0;i<cb->ipc_ptr->s_dat[sensor].channel_index;i++)
			if(i==0)
				printf("   %i",cb->ipc_ptr->s_dat[sensor].channel[i]);
			else
				printf(", %i",cb->ipc_ptr->s_dat[sensor].channel[i]);
		printf("\n\r");
	}
	printf("\n\r");
	strcpy(cb->prompt_buffer, "");

	/* build prompt */
	// c_34(cb);
	// strcpy(cmd_fsm_cb.prompt_buffer, " enter enter table number");
	return 0;
}





/**************** end command fsm action routines ******************/

/* cycle state machine */
void cmd_fsm(_CMD_FSM_CB * cb)
{
	int         num, index;


	// cb->token_type = cmd_type(cb->token);

// #if defined (_ATRACE) || defined (_FTRACE)
// 	sprintf(trace_buf, "cmd_fsm called: token <%s>, token value <%i>, token type <%i>, state <%i>, new token type <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state, token_type(cb->token));
// 	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
// #endif

	/* set up control block values based on token type */
	cb->token_type = token_type(cb->token);
	switch (cb->token_type) {
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
		if ((cb->token_value < 0) || (cb->token_value > _CMD_TOKENS )) {
			printf("\n\r****************************************\n\r");
			printf("**** command number out of range *******\n\r");
			printf("************** %2i **********************\n\r", cb->token_value);
			printf("****************************************\n\r");
			printf("\r\naborting application\n\r");
			term1();
		}
		index = cb->token_value;
	}

#if defined (_ATRACE) || defined (_FTRACE)
	sprintf(trace_buf, "cmd_fsm called before setting new state: index <%i>token <%s>, token value <%i>, token type <%i>, state <%i>\n", index, cb->token, cb->token_value, cb->token_type, cb->state);
	strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif

	if (cmd_action[index][cb->state](cb) == 0) {		// fire off a fsm action routine
		cb->p_state = cb->state;
		cb->state = cmd_new_state[index][cb->state];	// update state

#if defined (_ATRACE) || defined (_FTRACE)
		sprintf(trace_buf, "cmd_fsm called after setting new state: token <%s>, token value <%i>, token type <%i>, state <%i>\n", cb->token, cb->token_value, cb->token_type, cb->state);
		strace(_TRACE_FILE_NAME, trace_buf, trace_flag);
#endif
	}         //transition to next state
	else
	{
#if defined (_ATRACE) || defined (_FTRACE)
		printf("error returned from action routine\n\r");
#endif
		while (pop_cmd_q(cmd_fsm_cb.token)); //empty command queue
	}
	return;
}



