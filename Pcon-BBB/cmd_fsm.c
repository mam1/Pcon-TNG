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

/* schedule stuff */
int             sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE], *sch_ptr;
int             schlib[_MAX_SCHLIB_SCH][_SCHEDULE_SIZE];
char            schlib_name[_MAX_SCHLIB_SCH][_SCHEDULE_NAME_SIZE];
int             w_schedule_name[_SCHEDULE_NAME_SIZE];
int             w_schedule_number;
int             w_schedule[_SCHEDULE_SIZE];

/* channel data */
int             w_channel;                      //working channel number

/* working time */
int             w_minutes;
char            w_minutes_str[4];
int             w_hours;
char            w_hours_str[4];
// uint8_t         c_state[_NUMBER_OF_CHANNELS];   //channel state array, 0 = off, 1 = on


/***************************************/
/*****  command  parser fsm start ******/
/***************************************/

/* command list */
char    *keyword[_CMD_TOKENS] = {
/*  0 */    "INT",
/*  1 */    "STR",
/*  2 */    "OTHER",
/*  3 */    "OTHER",
/*  4 */    "q",
/*  5 */    "quit",
/*  6 */    "ping",
/*  7 */    "file",
/*  8 */    "done",
/*  9 */    "back",
/* 10 */    "new",
/* 11 */    "assign",
/* 12 */    "delete",
/* 13 */    "zero",
/* 14 */    "on",
/* 15 */    "off",
/* 16 */    "system",
/* 17 */    "status",
/* 18 */    "time",
/* 19 */    "t&s",
/* 20 */    "cycle",
/* 21 */    "startup",
/* 22 */    "reboot",
/* 23 */    "save",
/* 24 */    "schedule",
/* 25 */    "channel",
/* 26 */    "load",
/* 27 */    "help",
/* 28 */    "?"  };

/* command definitions */
char    *keyword_defs[_CMD_TOKENS] = {
/*  0 */    "valid integer",
/*  1 */    "alpha numeric string",
/*  2 */    "OTHER",
/*  3 */    "OTHER",
/*  4 */    "terminate the application",
/*  5 */    "quit",
/*  6 */    "ping",
/*  7 */    "set the real time clock",
/*  8 */    "end function",
/*  9 */    "revert to previous state",
/* 10 */    "create a new schedule",
/* 11 */    "assign a schedule to a (day,channel)",
/* 12 */    "delete schedule template",
/* 13 */    "zero",
/* 14 */    "turn channel on, set channel control mode to manual",
/* 15 */    "turn channel off, set channel control mode to manual"
/* 16 */    "display system data",
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
/* 27 */    "display all valid commands",
/* 28 */    "display all valid commands"  };

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] ={
/*                    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21*/
/*  0  INT      */  { 1,  1,  3,  0,  6,  6, 11,  8,  9, 10,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  1  STR      */  { 0,  0,  2,  3,  6,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  2  OTHER    */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  3  OTHER    */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  4  quit     */  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  5     q     */  { 0,  0,  0,  0,  0,  0,  0,  0,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  6  ping     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  7  clock    */  {13,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  8  done     */  { 0,  0,  0,  0,  0,  4,  4,  0,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/*  9  back     */  { 0,  0,  1,  2,  0,  0,  4,  4,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 10  new      */  { 0,  1,  2,  3,  5,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 11  assign   */  { 0,  1,  2,  3,  7,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 12  delete   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 13  zero     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 14  on       */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 15  off      */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 16  system   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 17  status   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 18  time     */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 19  t&s      */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 20  cycle    */  { 0,  2,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 21  startup  */  { 0,  0,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 22  reboot   */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 23  save     */  { 0,  1,  2,  3,  4,  5,  4,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 24  schedule */  { 4,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 25  channel  */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 26  load     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* 27  help     */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,  0,  0,  0,  0},
/* 28  ?        */  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  0,  0,  0,  0,  0}};

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
int c_18(CMD_FSM_CB *); /* enter schedule mode */
int c_19(CMD_FSM_CB *); /* set working schedule name */
int c_20(CMD_FSM_CB *); /* set working schedule hour */
int c_21(CMD_FSM_CB *); /* set working schedule minute */

/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
/*                STATE 0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21  */
/*  0  INT      */  { c_4,  c_7, c_16, c_17,  c_0,  c_0, c_20, c_21,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  1  STR      */  { c_7,  c_5,  c_0,  c_0, c_19,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  2  OTHER    */  { c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  3  OTHER    */  { c_8,  c_8,  c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  4  q        */  { c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_3,  c_0,  c_0,  c_0,  c_0},
/*  5  quit     */  { c_8,  c_8,  c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  6  ping     */  { c_2,  c_7,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  7  clock    */  { c_7,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  8  done     */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  9  back     */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 10  new      */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 11  assign   */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 12  delete   */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 13  zero     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 14  on       */  { c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 15  off      */  { c_0, c_10,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 16  system   */  {c_14, c_14, c_14,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 17  status   */  { c_6,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 18  time     */  { c_0, c_11,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 19  t&s      */  { c_7, c_12,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 20  cycle    */  { c_0, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 21  startup  */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 22  reboot   */  { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 23  save     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 24  schedule */  {c_18,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 25  channel  */  { c_8,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 26  load     */  { c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 27  help     */  { c_8,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/* 28  ?        */  { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0}};

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


int update_sch(char *sch_ptr){
    int             i;
    int             cmd = PING;
    int             ret = '\0';
    int             *size;
    uint8_t         s[4];

    size = (int *)&s[0];
    *size = sizeof(sch);
    printf("  sending ping <%u> to C3 \r\n",cmd);
    s_wbyte(bbb,&cmd);
    // printf("  ping <%u> sent\r\n",cmd);
    s_rbyte(bbb,&ret);
    // printf("<%u> returned from read\n", ret);
    if(ret == ACK){
        printf("  BBB acknowledge received <%u>\n\r",ret);
        printf("  send WRITE_CMD <%u> \r\n",WRITE_SCH);
        cmd = WRITE_SCH;
        s_wbyte(bbb,&cmd);
        printf("  sending schedule size <%i>\n\r",*size);
        for(i=0;i<4;i++){
            s_wbyte(bbb,(int *)&s[i]);
        }
        sch_ptr = &sch[0][0][0];
        printf("  sending schedule to the C3\r\n");
        for(i=0;i<*size;i++){
            s_wbyte(bbb,(int *)sch_ptr++);
        }
        return 0;

    }
    printf("  BBB  received <%u>\n\r",ret);

    return 1;
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
    for(i=0;i<(sizeof(keyword)/4);i++){
        // printf("i %i\r\n",i);
        if((strlen(keyword[i]) > dots)){
            dots = strlen(keyword[i]);
            // printf("dots %i\r\n",dots);            
        }
    }
    // printf("dots = %i\r\n",dots);
    
    printf("valid commands in state %i\r\n",cb->state);
    printf("  ESC key . resets the command processor to state 0 and clears all queues\r\n");
    for(i=0;i<_CMD_TOKENS;i++){
        if((cmd_action[i][cb->state] != c_8) && (cmd_action[i][cb->state] != c_7) && (cmd_action[i][cb->state] != c_0)){
            printf("  %s ",keyword[i]);
            for(ii=0;ii<(dots - strlen(keyword[i]));ii++){
                printf(".");
            }
            printf(" %s\r\n",keyword_defs[i]);
        }
    }
    /* build prompt */
    strcpy(cb->prompt_buffer,"\r\n> ");
    return 0;    
}
/* ping BBB */
int c_2(CMD_FSM_CB *cb)
{
    int             i, ii, iii, iiii;
	int             cmd = PING;
    int             ret = '\0';
    int             *size;
    uint8_t         s[4];
    // int             s;
    size = (int *)&s[0];
    *size = sizeof(sch);
	printf("  sending ping <%u> to C3 \r\n",cmd);
	s_wbyte(bbb,&cmd);
    // printf("  ping <%u> sent\r\n",cmd);
    s_rbyte(bbb,&ret);
    // printf("<%u> returned from read\n", ret);
    if(ret == ACK){
        printf("  BBB acknowledge received <%u>\n\r",ret);
        printf("  send WRITE_CMD <%u> \r\n",WRITE_SCH);
        cmd = WRITE_SCH;
        s_wbyte(bbb,&cmd);
        printf("  sending schedule size <%i>\n\r",*size);
        for(i=0;i<4;i++){
            s_wbyte(bbb,(int *)&s[i]);
        }
        printf("sending schedule\r\n");
        iiii=0;
        for(i=0;i<_DAYS_PER_WEEK;i++)
            for(ii=0;ii<_NUMBER_OF_CHANNELS;ii++)
                for(iii=0;iii<(_SCHEDULE_SIZE);iii++){
                    sch[i][ii][iii] = iiii++;
                }

        sch_ptr = &sch[0][0][0];
        for(i=0;i<*size;i++){
            s_wbyte(bbb,(int *)sch_ptr++);
        }
        return 0;

    }
    printf("  BBB  received <%u>\n\r",ret);
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
        w_channel = cb->token_value;
        w_minutes = 0;
        w_hours = 0;
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

    strcpy(sdat.c_data[w_channel].name,dequote(cb->token));
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    /* build prompt */
    strcpy(cb->prompt_buffer,"name set for channel ");
    sprintf(numstr, "%d", w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer,"\n\r> ");
    return 0;
}
/* display channel data */
int c_6(CMD_FSM_CB *cb)
{
    int         i;
    for(i=0;i<_NUMBER_OF_CHANNELS;i++){
        printf("  %s - %i %s - %s",onoff[sdat.c_data[i].c_state],i,c_mode[sdat.c_data[i].c_mode],sdat.c_data[i].name);
        if((sdat.c_data[i].c_mode) == 3)
            printf(" (%i:%i)",sdat.c_data[i].on_sec,sdat.c_data[i].off_sec);      
        printf("\r\n");
    }
    strcpy(cb->prompt_buffer,"\n\r> ");
    return 0;
}
/* command is not valid in current state */
int c_7(CMD_FSM_CB *cb)
{
    char        numstr[2];
    /* build prompt */
    strcpy(cb->prompt_buffer,"'");
    strcat(cb->prompt_buffer,cb->token);
    strcat(cb->prompt_buffer,"' is not a valid command in state ");
    sprintf(numstr, "%d", cb->state);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer,"\n\r> ");

    return 0;
}
/* command is not recognized */
int c_8(CMD_FSM_CB *cb)
{
    strcpy(cb->prompt_buffer,"'");
    strcat(cb->prompt_buffer,cb->token);
    strcat(cb->prompt_buffer,"' is not a valid command\n\r> ");
    return 0;
}
/* set channel control mode to manual and turn channel on */
int c_9(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[w_channel].c_mode = 0;
    sdat.c_data[w_channel].c_state = 1;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    /* build prompt */
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " turned on and mode set to manual\r\n> ");
    return 0;
}
/* set channel control mode to manual and turn channel off */
int c_10(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[w_channel].c_mode = 0;
    sdat.c_data[w_channel].c_state = 0;
    /* build prompt */
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " turned off and mode set to manual\r\n> ");
    return 0;
}
/* set channel control mode to time */
int c_11(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[w_channel].c_mode = 1;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " mode set to time\r\n> ");
    return 0;
}
/* set channel control mode to time and sensor */
int c_12(CMD_FSM_CB *cb)
{
    char        numstr[2];
    sdat.c_data[w_channel].c_mode = 2;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);
    strcpy(cb->prompt_buffer,"channel ");
    sprintf(numstr, "%d", w_channel);
    strcat(cb->prompt_buffer,numstr);
    strcat(cb->prompt_buffer, " mode set to time & sensor\r\n> ");
    return 0;
}
/* set channel control mode to cycle */
int c_13(CMD_FSM_CB *cb)
{
    char        sbuf[20];

    sdat.c_data[w_channel].c_mode = 3;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    strcpy(cb->prompt_buffer,"  setting cycle mode for channel ");
    sprintf(sbuf, "%d", w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer,"\r\n  enter on seconds > ");
    return 0;
}
/* display system data */
int c_14(CMD_FSM_CB *cb)
{

    disp_sys();
    strcpy(cb->prompt_buffer,"\r\n> ");

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

    sdat.c_data[w_channel].on_sec = cb->token_value;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    /* build prompt */    
    strcpy(cb->prompt_buffer,"  setting cycle mode for channel ");
    sprintf(sbuf, "%d", w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," on ");
    sprintf(sbuf, "%d", sdat.c_data[w_channel].on_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," seconds\r\n  enter off seconds > ");

    return 0;
}
/* set off cycle time */
int c_17(CMD_FSM_CB *cb)
{
    char            sbuf[20];  //max number of digits for a int

    sdat.c_data[w_channel].off_sec = cb->token_value;
    save_channel_data(_SYSTEM_DATA_FILE,&sdat);

    /* build prompt */
    strcpy(cb->prompt_buffer,"  channel ");
    sprintf(sbuf, "%d", w_channel);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer," mode set to cycle ");
    sprintf(sbuf, "%d", sdat.c_data[w_channel].on_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer,":");
    sprintf(sbuf, "%d", sdat.c_data[w_channel].off_sec);
    strcat(cb->prompt_buffer,sbuf);
    strcat(cb->prompt_buffer, " (on:off)\r\n\n> ");
    return 0;
}

/* enter schedule mode */
int c_18(CMD_FSM_CB *cb)
{
    int             i,hit;
    /* build prompt */
    strcpy(cb->prompt_buffer,"schedule maintenance\r\n");
    hit =0;
    for(i=0;i<_NUMBER_OF_CHANNELS;i++){
        if(schlib[i][0] != '\0'){
            hit = 1;
            strcat(cb->prompt_buffer,&schlib_name[i][0]);
            printf("  %i - %s\r\n",i,&schlib_name[i][0]);
        }
    }
    if(hit == 0) 
        strcat(cb->prompt_buffer,
            "  no schedule templates defined\r\n  enter name for new template\r\n  > "); 
    else
        strcat(cb->prompt_buffer,
            "  enter template number to edit or name of new template\r\n  > "); 
    return 0;
}

/* set working schedule name */
int c_19(CMD_FSM_CB *cb)
{
    strcpy(w_schedule_name,cb->token);
    dequote(w_schedule_name);

    /* build prompt */
    strcpy(cb->prompt_buffer,"  editing schedule template: ");
    strcat(cb->prompt_buffer,w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n  > ");
    return 0;
}

/* set working schedule hour */
int c_20(CMD_FSM_CB *cb)
{
    if((cb->token_value > 23)|| (cb->token_value < 0)){
        strcpy(cb->prompt_buffer,"  hour must be 0 - 23\r\n  enter hour > ");  
        return 1; 
    }

    w_hours = cb->token_value;
    strcpy(w_hours_str,cb->token);


    /* build prompt */
    strcpy(cb->prompt_buffer,"  editing schedule template: ");
    strcat(cb->prompt_buffer,w_schedule_name);
    strcat(cb->prompt_buffer, " ");
    strcat(cb->prompt_buffer, w_hours_str);
    strcat(cb->prompt_buffer, ":\r\n");
    strcat(cb->prompt_buffer,"  enter minute > ");
    return 0;
}

/* set working schedule minute */
int c_21(CMD_FSM_CB *cb)
{
    if((cb->token_value > 59)|| (cb->token_value < 0)){
        strcpy(cb->prompt_buffer,"  minute must be 0 - 59\r\n  enter minute > ");  
        return 1; 
    }

    w_minutes = cb->token_value;
    strcpy(w_minutes_str,cb->token);

    /* build prompt */
    strcpy(cb->prompt_buffer,"  editing schedule template: ");
    strcat(cb->prompt_buffer,w_schedule_name);
    strcat(cb->prompt_buffer,"\r\n  enter action for ");
    strcat(cb->prompt_buffer,w_hours_str);
    strcat(cb->prompt_buffer,":");
    strcat(cb->prompt_buffer,w_minutes_str);
    strcat(cb->prompt_buffer," >");
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
        // printf("*** error returned from action routine\n");
    }
    return;
}



