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
#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"
#include "serial_io.h"
#include "schedule.h"


/*********************** externals **************************/
 extern int             cmd_state,char_state;
 extern char            input_buffer[_INPUT_BUFFER_SIZE],*input_buffer_ptr;
 extern char            c_name[_CHANNEL_NAME_SIZE][_NUMBER_OF_CHANNELS];
 extern int 		    exit_flag;		              //exit man loop if TRUE
 extern int             trace_flag;                   //trace file is active
 extern int             bbb;				          //UART1 file descriptor
 extern CMD_FSM_CB      cmd_fsm_cb, *cmd_fsm_cb_ptr;  //cmd_fsm control block
 extern SYS_DAT         sdat;                         //system data structure


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

/***************************************/
/*****  command  parser fsm start ******/
/***************************************/

/* command list */
char    *keyword[_CMD_TOKENS] = {
/*  0 */    "INT",                                                                                                                        
/*  1 */    "STR",                                                                                                                              
/*  2 */    "",    //                                                                                                            
/*  3 */    "*",                                                                                                                                
/*  4 */    "",        //
/*  5 */    "schedule",                                                                                                 
/*  6 */    "ping",                                                                                 
/*  7 */    "",
/*  8 */    "",     //
/*  9 */    "",     //
/* 10 */    "",
/* 11 */    "edit",
/* 12 */    "delete",
/* 13 */    "zero",
/* 14 */    "on",
/* 15 */    "off",
/* 16 */    "",   //
/* 17 */    "status",
/* 18 */    "time",     
/* 19 */    "t&s",
/* 20 */    "cycle",
/* 21 */    "startup",
/* 22 */    "reboot",
/* 23 */    "save",
/* 24 */    "template",
/* 25 */    "channel",
/* 26 */    "load",
/* 27 */    "OTHER",
/* 28 */    "?",
/* 29 */    "q",
/* 30 */    "done",
/* 31 */    "back",
/* 32 */    "system",
/* 33 */    "debug" };         

/* command definitions */
char    *keyword_defs[_CMD_TOKENS] = {
/*  0 */    "valid integer",
/*  1 */    "alpha numeric string",
/*  2 */    "",
/*  3 */    "wild card match",
/*  4 */    "",
/*  5 */    "edit the system schedule",
/*  6 */    "send a ping to the C3",
/*  7 */    "",
/*  8 */    "",
/*  9 */    "",
/* 10 */    "",
/* 11 */    "edit system schedule",
/* 12 */    "delete schedule template",
/* 13 */    "zero",
/* 14 */    "turn channel on, set channel control mode to manual",
/* 15 */    "turn channel off, set channel control mode to manual",
/* 16 */    "",
/* 17 */    "display status for all channels",
/* 18 */    "set channel control mode to time",
/* 19 */    "set channel control mode to time and sensor",
/* 20 */    "set channel control mode to cycle",
/* 21 */    "startup",
/* 22 */    "reboot",
/* 23 */    "save schedule template",
/* 24 */    "edit schedule template library",
/* 25 */    "channel",
/* 26 */    "load",
/* 27 */    "unrecognized command",
/* 28 */    "display all valid commands",
/* 29 */    "terminate Pcon",
/* 30 */    "end function",
/* 31 */    "back up one state",
/* 32 */    "display system data",
/* 33 */    "display debug data" }; 

/* state specific command definitions for INT */
char    *INT_def[_CMD_STATES] = {
/*  0 */    "channel number",
/*  1 */    "",   
/*  2 */    "on seconds",
/*  3 */    "off seconds",
/*  4 */    "template number",
/*  5 */    "",
/*  6 */    "hour",
/*  7 */    "day number",
/*  8 */    "channel number",
/*  9 */    "template number",
/* 10 */    "",
/* 11 */    "minute",
/* 12 */    "",
/* 13 */    "hour",
/* 14 */    "minute",
/* 15 */    "second",
/* 16 */    "day of the week",
/* 17 */    "date",
/* 18 */    "month",
/* 19 */    "year",
/* 20 */    "",
/* 21 */    "" };

    /* state specific command definitions for STR */
char    *STR_def[_CMD_STATES] = {
/*  0 */    "",
/*  1 */    "channel name",   
/*  2 */    "",
/*  3 */    "",
/*  4 */    "template name",
/*  5 */    "",
/*  6 */    "",
/*  7 */    "",
/*  8 */    "",
/*  9 */    "",
/* 10 */    "",
/* 11 */    "",
/* 12 */    "",
/* 13 */    "",
/* 14 */    "",
/* 15 */    "",
/* 16 */    "",
/* 17 */    "",
/* 18 */    "",
/* 19 */    "",
/* 20 */    "",
/* 21 */    "" };

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] ={
/*                    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21*/
/*  0  INT      */  { 1,  1,  3,  0,  6,  6, 11,  8,  9, 10,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  1  STR      */  { 0,  0,  2,  3,  6,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  2  $        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  3  *        */  { 0,  1,  2,  3,  4,  5,  6,  8,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  4  $        */  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  5  schedule */  { 7,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  6  ping     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  7  $        */  {13,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  8  $        */  { 0,  0,  0,  0,  0,  4,  4,  0,  0,  0,  0,  6,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  9  $        */  { 0,  0,  1,  2,  0,  0,  4,  4,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 10  $        */  { 0,  1,  2,  3,  5,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 11  edit     */  { 0,  1,  2,  3,  7,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 12  delete   */  { 0,  1,  2,  3,  4,  5,  4,  7,  8,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 13  zero     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 14  on       */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 15  off      */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 16  $        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 17  status   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 18  time     */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 19  t&s      */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 20  cycle    */  { 0,  2,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 21  startup  */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 22  reboot   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 23  save     */  { 0,  1,  2,  3,  4,  5,  4,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 24  template */  { 4,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 25  channel  */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 26  load     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 27  OTHER    */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,  0,  0,  0,  0},
/* 28  ?        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21},
/* 29  q        */  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 30  done     */  { 0,  0,  0,  0,  0,  4,  4,  0,  0,  0,  0,  6,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 31  back     */  { 0,  0,  1,  2,  0,  0,  4,  4,  7,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 32  system   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21},
/* 33  debug    */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21}};

/*cmd processor functions */
int c_0(CMD_FSM_CB *); /* nop */
int c_1(CMD_FSM_CB *); /* display all valid commands for the current state */
int c_2(CMD_FSM_CB *); /* ping */
int c_3(CMD_FSM_CB *); /* terminate program */
int c_4(CMD_FSM_CB *); /* set working channel number*/
int c_5(CMD_FSM_CB *); /* set channel name for working channel */
int c_6(CMD_FSM_CB *); /* display channel data */
int c_7(CMD_FSM_CB *); /* command not valid in current state */ 
int c_8(CMD_FSM_CB *); /* command is not recognized */ 
int c_9(CMD_FSM_CB *); /* set channel control mode to manual and turn channel on */ 
int c_10(CMD_FSM_CB *); /* set channel control mode to manual and turn channel off */ 
int c_11(CMD_FSM_CB *); /* set channel control mode to time */ 
int c_12(CMD_FSM_CB *); /* set channel control mode to time & sensor */ 
int c_13(CMD_FSM_CB *); /* set channel control mode to cycle */ 
int c_14(CMD_FSM_CB *); /* display system data */ 
int c_15(CMD_FSM_CB *); /* revert to previous state */ 
int c_16(CMD_FSM_CB *); /* set on cycle time */ 
int c_17(CMD_FSM_CB *); /* set off cycle time */ 
int c_18(CMD_FSM_CB *); /* enter template maintenance mode */
int c_19(CMD_FSM_CB *); /* set working schedule name */
int c_20(CMD_FSM_CB *); /* set working schedule hour */
int c_21(CMD_FSM_CB *); /* set working schedule minute */
int c_22(CMD_FSM_CB *); /* set schedule record to on */
int c_23(CMD_FSM_CB *); /* set set schedule record to off */
int c_24(CMD_FSM_CB *); /* delete schedule record */
int c_25(CMD_FSM_CB *); /* save schedule template */
int c_26(CMD_FSM_CB *); /* delete schedule template */
int c_27(CMD_FSM_CB *); /* edit template */
int c_28(CMD_FSM_CB *); /* enter schedule maintenance mode */
int c_29(CMD_FSM_CB *); /* set working channel */
int c_30(CMD_FSM_CB *); /* set working day */
int c_31(CMD_FSM_CB *); /* set working channel to all */
int c_32(CMD_FSM_CB *); /* set working day to all */
int c_33(CMD_FSM_CB *); /* build new schedule */
int c_34(CMD_FSM_CB *); /* state 0 prompt */
int c_35(CMD_FSM_CB *); /* set working template number */
int c_36(CMD_FSM_CB *); /* append state 0 prompt to prompt buffer */
int c_37(CMD_FSM_CB *); /* display debug data */
int c_38(CMD_FSM_CB *); /* state 7 prompt */



/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
/*                STATE 0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21  */
/*  0  INT      */  { c_4,  c_7, c_16, c_17, c_27,  c_0, c_20, c_29, c_30, c_35, c_33, c_21,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  1  STR      */  { c_7,  c_5,  c_0,  c_0, c_19,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  2  $        */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  3  *        */  { c_8,  c_8,  c_8,  c_0,  c_0,  c_0,  c_0, c_31, c_32,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  4  $        */  { c_0,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_0,  c_0,  c_0,  c_0},
/*  5  schedule */  {c_28,  c_8,  c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  6  ping     */  { c_2,  c_7,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  7  clock    */  { c_7,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  8  $        */  { c_0, c_34,  c_0,  c_0, c_34,  c_0, c_18, c_34,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  9  $        */  { c_0, c_34,  c_0,  c_0, c_34,  c_0,  c_0, c_34,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 10  $        */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 11  edit     */  { c_7,  c_0,  c_0,  c_0, c_28,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 12  delete   */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0, c_26,  c_0,  c_0,  c_0,  c_0,  c_0, c_24,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 13  zero     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 14  on       */  { c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_7,  c_0,  c_0,  c_0,  c_0,  c_7, c_22,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 15  off      */  { c_0, c_10,  c_0,  c_0,  c_0,  c_0,  c_7,  c_0,  c_0,  c_0,  c_0,  c_7, c_23,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 16  $        */  { c_0, c_14, c_14,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 17  status   */  { c_6,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 18  time     */  { c_0, c_11,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 19  t&s      */  { c_7, c_12,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 20  cycle    */  { c_0, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 21  startup  */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 22  reboot   */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 23  save     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0, c_25,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 24  template */  {c_18,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 25  channel  */  { c_8,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 26  load     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 27  OTHER    */  { c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 28  ?        */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1},
/* 29  q        */  { c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_0,  c_0,  c_0,  c_0},
/* 30  done     */  { c_0, c_34,  c_0,  c_0, c_34,  c_0, c_18, c_34, c_34, c_34, c_34,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 31  back     */  { c_0, c_34,  c_0,  c_0, c_34,  c_0,  c_0, c_34,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 32  system   */  {c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14, c_14},
/* 33  debug    */  {c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37, c_37}};

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

    if((*c=='\0') || (*c==' '))
        return 3;
    /* test for a quoted string*/
    if(*c==_QUOTE)
        return 1;
    /* test for a integer */
    if(is_valid_int(c))
        return 0;
    /* test for a keyword */
    for(i=3;i<_CMD_TOKENS;i++)
    {
        if(strlen(c) == strlen(keyword[i])){
            p = c;
            while(*p != '\0'){
                *p = tolower(*p);
                p++;
            }; 
            if(strncmp(c,keyword[i],strlen(c))==0)
                return i;
        }
    }
    /* unrecognized token */
    return 2;
}

char *dequote(char *s){
    char        *p1,*p2;
    p1=p2=s;
    while(*p1 != '\0'){
        if(*p1 == _QUOTE) p1++;
        else *p2++ = *p1++;
    }
    *p2 = '\0';
    return s;
}

/* reset cmd_fsm to initial state */
void cmd_fsm_reset(CMD_FSM_CB *cb){
    memset(&cb->prompt_buffer,'\0',sizeof(cb->prompt_buffer));
    strcpy(cb->prompt_buffer,"\n\r> ");
    cb->state = 0;
    return;
}

/* load buffer with a list of all records in a schedule */
char *sch2text(uint32_t *sch,char *buf){
    int         sch_recs,  i,key, h, m;

    /*build list of schedule records for prompt */
    *buf = '\0';
    sch_recs = *sch;
    if(sch_recs == 0)
        strcat(buf,"  no schedule records");
    else
        for(i=1;i < sch_recs+1;i++){
            key = get_key(sch[i]);
            h = key / 60;
            m = key % 60;
            sprintf(&buf[strlen(buf)],"  %2i:%02i ",h,m);
            strcat(buf,onoff[get_s(sch[i])]);
            strcat(buf,"\n\r");
        }
    return buf;
}

/* load schedule template list into buffer  */
char *sch2text2(uint32_t *sch,char *buf){
    int         sch_recs,  i,key, h, m;

    /*build list of schedule */
    *buf = '\0';
    sch_recs = *sch;

    if(sch_recs == 0)
        strcat(buf,"  no schedule records");
    else
        for(i=1;i < sch_recs+1;i++){

            key = get_key(sch[i]);
            h = key / 60;
            m = key % 60;
            sprintf(&buf[strlen(buf)],"  %2i:%02i ",h,m);
            strcat(buf,onoff[get_s(sch[i])]);
        }
    return buf;
}

/* append schedule template list to buffer  */
char *make_lib_list(char *buf, CMD_FSM_CB *cb){

    int             i,ii;
    int             max_name_size;
    char            pad[_SCHEDULE_NAME_SIZE];
    int             pad_size;
    char            sbuf[128];

    if(cb->sdat_ptr->schlib_index == 0){
        strcat(cb->prompt_buffer,
            "  no schedule templates defined\r\n  enter name, in quotes, to create a new template\r\n  > "); 
        return buf;
    }

    // hit = 0;
    for(i=0;i<cb->sdat_ptr->schlib_index+1;i++){
        if(cb->sdat_ptr->s_data[i].name[0] != '\0'){
            // hit = 1;

            max_name_size = 0;
            for(i=0;i<cb->sdat_ptr->schlib_index;i++)
                if(max_name_size < strlen(cb->sdat_ptr->s_data[i].name))
                    max_name_size = strlen(cb->sdat_ptr->s_data[i].name);

            for(i=0;i<cb->sdat_ptr->schlib_index;i++){
                pad_size = max_name_size - strlen(cb->sdat_ptr->s_data[i].name);
                pad[0] = '\0';
                for(ii=0;ii<pad_size;ii++)
                    strcat(pad," ");
                // printf("    %i - %s%s  %s\r\n",i,cb->sdat_ptr->s_data[i].name,pad,sch2text2(cb->sdat_ptr->s_data[i].schedule,buf));
                sprintf(&cb->prompt_buffer[strlen(cb->prompt_buffer)],"    %i - %s%s  %s\r\n",i,cb->sdat_ptr->s_data[i].name,pad,sch2text2(cb->sdat_ptr->s_data[i].schedule,sbuf));
            }
        }
    }
    // if(hit == 0) 
    //     strcat(cb->prompt_buffer,
    //         "  no schedule templates defined\r\n  enter name, in quotes, to create a new template\r\n  > "); 
    // else
    strcat(cb->prompt_buffer,
            "  enter template number to edit or name to create a new template\r\n  > "); 
    return buf;
}
/* print schedule template list */
void print_tlist(CMD_FSM_CB *cb){
    int             i,ii;
    int             max_name_size;
    char            pad[_SCHEDULE_NAME_SIZE];
    int             pad_size;
    char            buf[128];

    max_name_size = 0;
    for(i=0;i<cb->sdat_ptr->schlib_index;i++)
        if(max_name_size < strlen(cb->sdat_ptr->s_data[i].name))
            max_name_size = strlen(cb->sdat_ptr->s_data[i].name);

    for(i=0;i<cb->sdat_ptr->schlib_index;i++){
        pad_size = max_name_size - strlen(cb->sdat_ptr->s_data[i].name);
        pad[0] = '\0';
        for(ii=0;ii<pad_size;ii++)
            strcat(pad," ");
        printf("    %i - %s%s  %s\r\n",i,cb->sdat_ptr->s_data[i].name,pad,sch2text2(cb->sdat_ptr->s_data[i].schedule,buf));
    }

    return;
}

/**************** start command fsm action routines ******************/
/* do nothing */
int c_0(CMD_FSM_CB *cb)
{
    cb->prompt_buffer[0] = '>';
    cb->prompt_buffer[1] = ' ';
    cb->prompt_buffer[2] = '\0';
    return 0;
}
/* display all valid commands for the current state */
int c_1(CMD_FSM_CB *cb)
{
    int         i,ii;
    int         dots;

    // printf("sizeofkeyword %i\n\r",(sizeof(keyword)/4));

    printf("\r\ncurrent state %i\r\n",cb->state);

    printf("\r\nvalid commands any state\r\n  ESC .... reset command processor\r\n");
    printf("  ? ...... display current state and list all valid commands\r\n");
    printf("  q ...... terminate the application\r\n");
    printf("  system . display system data\r\n");
    printf("  done ... terminate active function\r\n");
    printf("  back ... return to previous state\r\n");

    printf("       -------------------------------------\r\n");

    for(i=0;i<_CMD_TOKENS;i++){
        // printf("keyword = %s, length = %i\r\n",keyword[i],strlen(keyword[i]));
        if((strlen(keyword[i]) > dots)){
            dots = strlen(keyword[i]);
            // printf("dots %i\r\n",dots);            
        }
    }
    // printf("dots = %i\r\n",dots);
    
    printf("valid commands in state %i\r\n",cb->state);
    for(i=0;i<_CMD_TOKENS - 7;i++){
        // if(cb->token_type != )
        if((cmd_action[i][cb->state] != c_8) && (cmd_action[i][cb->state] != c_7) && (cmd_action[i][cb->state] != c_0)){
            printf("  %s ",keyword[i]);
            for(ii=0;ii<((dots + 2)- strlen(keyword[i]));ii++){
                printf(".");
            }
            printf(" %s",keyword_defs[i]);
            if(i==0)
                printf(" (%s)",INT_def[i]);
            printf("\r\n");

        }
    }
    /* build prompt */
    c_34(cb);  // state 0 prompt
    return 0;    
}
/* ping BBB */
int c_2(CMD_FSM_CB *cb)
{
    // int             i, ii, iii, iiii;
 //  int             i;
	// int             cmd = _PING;
 //  int             ret = '\0';
 //  int             *size;
 //  uint8_t         s[4];
  // int             s;

 //  // size = sizeof(cb->sdat_ptr->sch);
	// printf("  sending ping <%u> to C3 \r\n",cmd);
	// s_wbyte(bbb,&cmd);
 //    // printf("  ping <%u> sent\r\n",cmd);
 //  s_rbyte(bbb,&ret);
 //    // printf("<%u> returned from read\n", ret);
 //  if(ret == _ACK){
 //      printf("  BBB acknowledge received <%u>\n\r",ret);
 //      printf("  send WRITE_CMD <%u> \r\n",_WRITE_SCH);
 //      cmd = _WRITE_SCH;
 //      s_wbyte(bbb,&cmd);
 //      printf("  sending schedule size <%i>\n\r",*size);
 //      for(i=0;i<4;i++){
 //          s_wbyte(bbb,(int *)&s[i]);
      // }
        // printf("sending schedule\r\n");
        // iiii=0;
        // for(i=0;i<_DAYS_PER_WEEK;i++)
        //     for(ii=0;ii<_NUMBER_OF_CHANNELS;ii++)
        //         for(iii=0;iii<(_SCHEDULE_SIZE);iii++){
        //             cb->sdat_ptr->sch[i][ii][iii] = iiii++;
        //         }

        // cb->sch_ptr = (uint32_t *)cb->sdat_ptr->sch;
        // for(i=0;i<*size;i++){
        //     s_wbyte(bbb,(int *)cb->sch_ptr++);
        // }
        // return 0;

    // }
  // printf("  BBB  received <%u>\n\r",ret);
	return 1;
}
/* terminate program */
int c_3(CMD_FSM_CB *cb)
{
	term(1);
    return 0;
}
/* set working channel number */
int c_4(CMD_FSM_CB *cb)
{
    if (cb->token_value < _NUMBER_OF_CHANNELS){
        cb->w_channel = cb->token_value;
       	cb->w_minutes = 0;
       	cb->w_hours = 0;
        strcpy(cb->prompt_buffer,"enter action for channel ");
        strcat(cb->prompt_buffer,cb->token);
        strcat(cb->prompt_buffer,"\n\r> ");
        return 0;
    }
    strcpy(cb->prompt_buffer,"channel number must be 0 to 7\r\n> ");
    return 1;
}
/* set channel name for working channel */
int c_5(CMD_FSM_CB *cb)
{
    char        numstr[2];

    strcpy(sdat.c_data[cb->w_channel].name,dequote(cb->token));
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    /* build prompt */
    strcpy(cb->prompt_buffer,"name set for channel ");
    sprintf(numstr, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer,"\r\n");
    c_36(cb);   //append state 0 prompt to prompt buffer
    return 0;
}
/* display channel data */
int c_6(CMD_FSM_CB *cb)
{
    int         i;
    for(i=0;i<_NUMBER_OF_CHANNELS;i++){
        // printf("state <%i>\r\n",sdat.c_data[i].c_state);
        printf("  %s - %i %s - %s",onoff[sdat.c_data[i].c_state],i,c_mode[sdat.c_data[i].c_mode],sdat.c_data[i].name);
        if((sdat.c_data[i].c_mode) == 3)
            printf(" (%i:%i)",sdat.c_data[i].on_sec,sdat.c_data[i].off_sec);      
        printf("\r\n");
    }
    c_34(cb);  // state 0 prompt
    return 0;
}
/* command is not valid in current state */
int c_7(CMD_FSM_CB *cb)
{
    char        numstr[2];
    char        hold_prompt[_PROMPT_BUFFER_SIZE];
    /* build prompt */
    strcpy(hold_prompt,cb->prompt_buffer);
    strcpy(cb->prompt_buffer,"'");
    strcat(cb->prompt_buffer,cb->token);
    strcat(cb->prompt_buffer,"' is not a valid command in state ");
    sprintf(numstr, "%d", cb->state);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer,"\n\r");
    strcat(cb->prompt_buffer,hold_prompt);

    return 1;
}
/* command is not recognized */
int c_8(CMD_FSM_CB *cb)
{
    strcpy(cb->prompt_buffer,"'");
    strcat(cb->prompt_buffer,cb->token);
    strcat(cb->prompt_buffer,"' is not a valid command\n\r> ");
    return 1;
}
/* set channel control mode to manual and turn channel on */
int c_9(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[cb->w_channel].c_mode = 0;
    sdat.c_data[cb->w_channel].c_state = 1;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    /* build prompt */
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " turned on and mode set to manual\r\n");
    c_36(cb);   //append state 0 prompt to prompt buffer

    return 0;
}
/* set channel control mode to manual and turn channel off */
int c_10(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[cb->w_channel].c_mode = 0;
    sdat.c_data[cb->w_channel].c_state = 0;
    /* build prompt */
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " turned off and mode set to manual\r\n");
    c_36(cb);   //append state 0 prompt to prompt buffer

    return 0;
}
/* set channel control mode to time */
int c_11(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[cb->w_channel].c_mode = 1;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " mode set to time\r\n");
    c_36(cb);   //append state 0 prompt to prompt buffer

    return 0;
}
/* set channel control mode to time and sensor */
int c_12(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[cb->w_channel].c_mode = 2;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " mode set to time & sensor\r\n");
    c_36(cb);   //append state 0 prompt to prompt buffer
    return 0;
}
/* set channel control mode to cycle */
int c_13(CMD_FSM_CB *cb)
{
    char        sbuf[20];

    sdat.c_data[cb->w_channel].c_mode = 3;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    strcpy(cb->prompt_buffer,"  setting cycle mode for channel ");
    sprintf(sbuf, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer,"\r\n  enter on seconds > ");
    return 0;
}
/* display system data */
int c_14(CMD_FSM_CB *cb)
{
    // char            temp_buf[128];

    printf("\r\ncurrent state %i\r\n\n",cb->state);
    printf("\r\n******* system data ***************************************************************\r\n\n");
    disp_sys();
    printf("\r\n******* schedule templates ********************************************************\r\n\n");
    print_tlist(cb);
    printf("\r\n******* system schedule ***********************************************************\r\n");
    disp_all_schedules(cb,(uint32_t *)cb->sdat_ptr->sch_ptr);

    c_34(cb);   // state 0 prompt
    return 0;
}
/* move back to previous state */
int c_15(CMD_FSM_CB *cb)
{

    cb->state = cb->p_state;
    strcpy(cb->prompt_buffer,"\r\n> ");

    return 0;
}
/* set on cycler time */
int c_16(CMD_FSM_CB *cb)
{
    char            sbuf[20];  //max number of digits for a int

    sdat.c_data[cb->w_channel].on_sec = cb->token_value;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    /* build prompt */    
    strcpy(cb->prompt_buffer,"  setting cycle mode for channel ");
    sprintf(sbuf, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," on ");
    sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].on_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," seconds\r\n  enter off seconds > ");

    return 0;
}
/* set off cycle time */
int c_17(CMD_FSM_CB *cb)
{
    char            sbuf[20];  //max number of digits for a int

    sdat.c_data[cb->w_channel].off_sec = cb->token_value;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    /* build prompt */
    strcpy(cb->prompt_buffer,"  channel ");
    sprintf(sbuf, "%d", cb->w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," mode set to cycle ");
    sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].on_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer,":");
    sprintf(sbuf, "%d", sdat.c_data[cb->w_channel].off_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer, " (on:off)\r\n\n> ");
    return 0;
}

/* enter template maintenance mode */
int c_18(CMD_FSM_CB *cb)
{

    /* build prompt */
    strcpy(cb->prompt_buffer,"schedule template maintenance\r\n\ntemplate library\r\n");
    make_lib_list(cb->prompt_buffer, cb);
    return 0;
}

/* set working schedule name */
int c_19(CMD_FSM_CB *cb)
{
    strcpy((char *)cb->w_schedule_name,cb->token);
    dequote((char *)cb->w_schedule_name);

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n  enter time (HH,MM) > ");
    return 0;
}

/* set working schedule hour */
int c_20(CMD_FSM_CB *cb)
{
    if((cb->token_value > 23)|| (cb->token_value < 0)){
        strcpy(cb->prompt_buffer,"  hour must be 0 - 23\r\n  enter hour > ");  
        return 1; 
    }

    cb->w_hours = cb->token_value;
    strcpy(cb->w_hours_str,cb->token);

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer, " ");
    strcat(cb->prompt_buffer, cb->w_hours_str);
    strcat(cb->prompt_buffer, ":\r\n");
    strcat(cb->prompt_buffer,"  enter minute > ");
    return 0;
}

/* set working schedule minute */
int c_21(CMD_FSM_CB *cb)
{
    char            temp[200];

    /* check value */
    if((cb->token_value > 59)|| (cb->token_value < 0)){
        strcpy(cb->prompt_buffer,"  minute must be 0 - 59\r\n  enter minute > ");  
        return 1; 
    }
    /* update schedule record */
    cb->w_minutes = cb->token_value;
    strcpy(cb->w_minutes_str,cb->token);

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n");
    strcat(cb->prompt_buffer,sch2text(cb->w_schedule,temp));
    strcat(cb->prompt_buffer,"\r\n  enter action for ");
    strcat(cb->prompt_buffer,cb->w_hours_str);
    strcat(cb->prompt_buffer,":");
    strcat(cb->prompt_buffer,cb->w_minutes_str);
    strcat(cb->prompt_buffer," > ");
    return 0;
}

/* set schedule record to on */
int c_22(CMD_FSM_CB *cb)
{
    char            temp[200];
    /*set the state of the schedule record to on */
    cb->w_srec_state = 1;       //set working state to on
    if(add_sch_rec(&cb->w_schedule[0], make_key(cb->w_hours,cb->w_minutes), 1)) // add/change record
        return -1;

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n");
    strcat(cb->prompt_buffer,sch2text(cb->w_schedule,temp));
    // strcat(cb->prompt_buffer,"\r\n  enter action for ");
    // strcat(cb->prompt_buffer,cb->w_hours_str);
    // strcat(cb->prompt_buffer,":");
    // strcat(cb->prompt_buffer,cb->w_minutes_str);
    strcat(cb->prompt_buffer,"\r\n  enter time (HH,MM) > ");
    return 0;
}

/* set set schedule record to off */
int c_23(CMD_FSM_CB *cb)
{
    // int                 sch_recs;
    char            temp[200];

    cb->w_srec_state = 0;       // set working state to off
    add_sch_rec(&cb->w_schedule[0], make_key(cb->w_hours,cb->w_minutes), 0);

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n");
    strcat(cb->prompt_buffer,sch2text(cb->w_schedule,temp));
    strcat(cb->prompt_buffer,"\r\n  enter time (HH,MM) > ");
    return 0;
}

/* delete schedule record */
int c_24(CMD_FSM_CB *cb)
{

    char            temp[200];

    del_sch_rec(&cb->w_schedule[0], make_key(cb->w_hours,cb->w_minutes));

    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n");
    strcat(cb->prompt_buffer,sch2text(cb->w_schedule,temp));
    strcat(cb->prompt_buffer,"\r\n  enter time (HH,MM) > ");
    return 0;
}

/* save schedule template */
int c_25(CMD_FSM_CB *cb)
{
    int             i, index;

    
    if(cb->sdat_ptr->schlib_index ==  0){
        index = 0;
        cb->sdat_ptr->schlib_index += 1;
    }
    else
        for(i=0;i<cb->sdat_ptr->schlib_index + 1;i++){
            if(strcmp(cb->sdat_ptr->s_data[i].name,(char *)cb->w_schedule_name) == 0){
                index = i;
                break;
            }
        }
    if((index != 0) && (index != i)){
        index = cb->sdat_ptr->schlib_index;
        cb->sdat_ptr->schlib_index += 1;
    } 

    strcpy(cb->sdat_ptr->s_data[index].name, (char *)cb->w_schedule_name);      //copy name
    for(i=0;i<_SCHEDULE_SIZE;i++){
        cb->sdat_ptr->s_data[index].schedule[i]  = cb->w_schedule[i];   //copy schedule
        cb->w_schedule[i] = '\0';                                       //clear working shcedule
    }
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    
    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\nschedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer," is saved\r\n\n");
    strcat(cb->prompt_buffer,"schedule maintenance\r\n");
    make_lib_list(cb->prompt_buffer, cb);

    return 0;
}

/* delete schedule template */
int c_26(CMD_FSM_CB *cb)
{

    // char            temp[200];
    int                i,ii;

    // printf("wipe out w index %i\r\n",cb->w_template_index);
    // printf("wipe out d index %i\r\n",cb->sdat_ptr->schlib_index);

    if(cb->w_template_index == (cb->sdat_ptr->schlib_index - 1)){                                    //delete high entry
        cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index -1;                                 //back down index
        memset(cb->sdat_ptr->s_data[cb->w_template_index].name,'\0',
            sizeof(cb->sdat_ptr->s_data[cb->w_template_index].name));                               //wipe name          
        memset(cb->sdat_ptr->s_data[cb->w_template_index].schedule,'\0',
            sizeof(cb->sdat_ptr->s_data[cb->w_template_index].schedule));                           //wipe schedule
        // cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index -1;                                 //back down index

    }
    else{
        for(i = cb->w_template_index;i < (cb->sdat_ptr->schlib_index); i++){
            strcpy(cb->sdat_ptr->s_data[i].name, cb->sdat_ptr->s_data[i+1].name);                   //copy name
            for(ii=0;ii<_SCHEDULE_SIZE;ii++){
                cb->sdat_ptr->s_data[i].schedule[ii] = cb->sdat_ptr->s_data[ii+1].schedule[ii];     //copy schedule
            }
        }
        cb->sdat_ptr->schlib_index = cb->sdat_ptr->schlib_index -1;                                 //back down index

    }
    cb->w_template_index = cb->w_template_index - 1;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

/* build prompt */
    strcpy(cb->prompt_buffer,"\r\nschedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer," deleted\r\n\n");
    strcat(cb->prompt_buffer,"schedule maintenance\r\n");
    make_lib_list(cb->prompt_buffer, cb);

    return 0;
}

/* edit schedule template */
int c_27(CMD_FSM_CB *cb)
{

    char            temp[200];
    int             i;

    cb->w_template_index = cb->token_value;
    strcpy((char *)cb->w_schedule_name, cb->sdat_ptr->s_data[cb->w_template_index].name);

    for(i=0;i<_SCHEDULE_SIZE;i++){
        cb->w_schedule[i]  = cb->sdat_ptr->s_data[cb->w_template_index].schedule[i];   //load schedule
    }
    strcpy((char *)cb->w_schedule_name, cb->sdat_ptr->s_data[cb->w_template_index].name);      //load name



    /* build prompt */
    strcpy(cb->prompt_buffer,"editing schedule template: ");
    strcat(cb->prompt_buffer,(char *)cb->w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n");
    strcat(cb->prompt_buffer,sch2text(cb->w_schedule,temp));
    // strcat(cb->prompt_buffer,"\r\n  enter action for ");
    // strcat(cb->prompt_buffer,cb->w_hours_str);
    // strcat(cb->prompt_buffer,":");
    // strcat(cb->prompt_buffer,cb->w_minutes_str);
    strcat(cb->prompt_buffer,"\r\n  enter time (HH,MM) > ");
    return 0;
}

/* enter schedule build mode */
int c_28(CMD_FSM_CB *cb)
{
    int             i, ii;
    int             max_name_size;       
    char            buf[128];
    char            pad[_SCHEDULE_NAME_SIZE];
    int             pad_size;

    max_name_size = 0;
    for(i=0;i<cb->sdat_ptr->schlib_index;i++)
        if(max_name_size < strlen(cb->sdat_ptr->s_data[i].name))
            max_name_size = strlen(cb->sdat_ptr->s_data[i].name);

    printf("\r\nediting system schedule\r\n\ntemplate library\r\n");
    for(i=0;i<cb->sdat_ptr->schlib_index;i++){
        pad_size = max_name_size - strlen(cb->sdat_ptr->s_data[i].name);
        pad[0] = '\0';
        for(ii=0;ii<pad_size;ii++)
            strcat(pad," ");
        printf("    %i - %s%s  %s\r\n",i,cb->sdat_ptr->s_data[i].name,pad,sch2text2(cb->sdat_ptr->s_data[i].schedule,buf));
    }

    /* load working schedule from system schedule */
    // printf(" size of schedule buffer = %i\r\n",sizeof(cmd_fsm_cb.w_sch));
    memcpy(cmd_fsm_cb.w_sch_ptr,cmd_fsm_cb.sdat_ptr->sch_ptr,sizeof(cmd_fsm_cb.w_sch));
    printf("\r\nsystem schedule copied to edit buffer\r\n");
    printf("copy of the system schedule\r\n");
    disp_all_schedules(cb,(uint32_t *)cb->w_sch);

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter channel{N(0...7)|*},day{N(1...7)|*},template{N}  > ");
    return 0;
}

/* set working channel */
int c_29(CMD_FSM_CB *cb)
{

    // printf("editing system schedule\r\n");
    cb->w_channel = cb->token_value;

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter day number or *  > ");
    return 0;
}

/* set working day */
int c_30(CMD_FSM_CB *cb)
{
    if ((cb->token_value > 0) && (cb->token_value < _DAYS_PER_WEEK + 1)){
 		cb->w_day = cb->token_value - 1;
        return 0;
    }
    strcpy(cb->prompt_buffer,"\r\n  day number must be 1 to 7 > ");
    return 1;

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter day number or * > ");

    return 0;

}

/* set working channel to all */
int c_31(CMD_FSM_CB *cb)
{
    // printf("editing system schedule\r\n");   
    cb->w_channel = _ALL_CHANNELS;
    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter day number or * > ");
    return 0;
}

/* set working day to all */
int c_32(CMD_FSM_CB *cb)
{
    // printf("editing system schedule\r\n");
    cb->w_day = _ALL_CHANNELS;

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter template number  > ");
    return 0;
}

/* build new schedule */
int c_33(CMD_FSM_CB *cb)
{
    int             channel, template, day;
    // int              i, ii, iii;

    template = cb->w_template_num;
    day = cb->w_day;
    channel = cb->w_channel;
    if(cb->w_channel == _ALL_CHANNELS)
      printf("  setting all channels ");
    else{
      channel = cb->w_channel;
      printf("setting channel %i ", cb->w_channel);
    }
    if(cb->w_day == _ALL_DAYS)
      printf("all days to schedule template %i\r\n",cb->w_template_num);
    else{
      day = cb->w_day;
      printf("day %i to schedule template %i\r\n",cb->w_day, cb->w_template_num);
    }

    if((cb->w_channel == _ALL_CHANNELS) && (cb->w_day == _ALL_DAYS)){
      for(day = 0;day < _DAYS_PER_WEEK; day++)
        for(channel = 0;channel < _NUMBER_OF_CHANNELS; channel++)
          load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer with 0 - _SCHEDULE_BUFFER
    }
    else if(cb->w_day == _ALL_DAYS)
        for(day = 0;day < _DAYS_PER_WEEK; day++)
          load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer with 0 - _SCHEDULE_BUFFER
    else if(cb->w_channel == _ALL_CHANNELS)
        for(channel = 0;channel < _NUMBER_OF_CHANNELS; channel++)
          load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer with 0 - _SCHEDULE_BUFFER
    else
     load_schedule(cb->w_sch_ptr, cb->sdat_ptr->s_data[template].schedule, day, channel);   // load schedule buffer with 0 - _SCHEDULE_BUFFER
 
    disp_all_schedules(cb,(uint32_t *)cb->w_sch);

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n    > ");
    return 0;
}

/* state 0 prompt */
int c_34(CMD_FSM_CB *cb)
{


    /* build prompt */
    strcpy(cmd_fsm_cb.prompt_buffer,"\r\nenter a command\r\n> ");
    return 0;
}

/* set working template number */
int c_35(CMD_FSM_CB *cb)
{


    cb->w_template_num = cb->token_value;
    c_33(cb);

    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n  enter template number  > ");
    return 0;
}

/* appenf state 0 prompt to prompt buffer */
int c_36(CMD_FSM_CB *cb)
{


    /* build prompt */
    strcat(cmd_fsm_cb.prompt_buffer,"\r\nenter a command\r\n> ");
    return 0;
}

/* display debug data */
int c_37(CMD_FSM_CB *cb)
{
    // char            temp_buf[128];

    printf("\r\ncurrent state %i\r\n",cb->state);
    printf("\r\n******* system data ***************************************************************\r\n\n");
    disp_sys();
    printf("\r\n******* schedule templates ********************************************************\r\n\n");
    print_tlist(cb);
    printf("\r\n******* system schedule ***********************************************************\r\n");
    disp_all_schedules(cb,(uint32_t *)cb->sdat_ptr->sch_ptr);
    printf("\r\n******* working schedule **********************************************************\r\n");
    disp_all_schedules(cb,(uint32_t *)cb->w_sch);

    c_34(cb);   // state 0 prompt
    return 0;
}

/* reset work variables and display state 7 prompt */ 
int c_38(CMD_FSM_CB *cb)
{
    // char            temp_buf[128];

   cb->w_channel = 0;
   cb->w_channel = 0;
   cb->w_template_num = 0;

   /* build prompt */
   c_27(cb);
   
   return 0; 
}
/**************** end command fsm action routines ******************/

/* cycle state machine */
void cmd_fsm(CMD_FSM_CB *cb)
{
    static int         num;
    // static char        *s_ptr;

    cb->token_type = cmd_type(cb->token);
#ifdef _TRACE
		    sprintf(trace_buf, "cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",cb->token,cb->token_type, cb->state);
			strace(_TRACE_FILE_NAME,trace_buf,trace_flag);
#endif
//    printf("cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",cb->token,cb->token_type, *state);
    if((cb->token_type==1)||(cb->token_type==2))
    {
        // n_ptr = NULL;
        // s_ptr = cb->token;
    }
    else if(cb->token_type==0) //integer
    {
        sscanf(cb->token,"%u",&num);
        cb->token_value = num;
        // n_ptr = &num;
        // s_ptr = NULL;
    }
    else
    {
        num = cb->token_type;
        // n_ptr = &num;
        // s_ptr = NULL;
    }
//    printf("call cmd_action[%i][%i](<%i>,<%i>,<%s>)\n",cb->token_type,*state,cb->token_type,*n_ptr,s_ptr);
    if(cmd_action[cb->token_type][cb->state](cb)==0){   //fire off an fsm action routine
        cb->p_state = cb->state;
        cb->state = cmd_new_state[cb->token_type][cb->state];
    }         //transition to next state
    else
    {
        while(pop_cmd_q(cmd_fsm_cb.token));  //empty command queue
    }
    return;
}



