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

/* code to text conversion */
extern char *day_names_long[7];
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];
/*********************** globals **************************/
#ifdef _TRACE
	char			trace_buf[128];
#endif

/***************************************/
/*****  command  parser fsm start ******/
/***************************************/

/* key word list */
char    *keyword[_CMD_TOKENS] = {
/*  0 */    "INT",
/*  1 */    "STR",
/*  2 */    "CMD",
/*  3 */    "OTHER",
/*  4 */    "q",
/*  5 */    "quit",
/*  6 */    "ping",
/*  7 */    "file",
/*  8 */    "edit",
/*  9 */    "quit",
/* 10 */    "cancel",
/* 11 */    "name",
/* 12 */    "mode",
/* 13 */    "zero",
/* 14 */    "on",
/* 15 */    "off",
/* 16 */    "system",
/* 17 */    "status",
/* 18 */    "time",
/* 19 */    "set",
/* 20 */    "shutdown",
/* 21 */    "startup",
/* 22 */    "reboot",
/* 23 */    "save",
/* 24 */    "schedule",
/* 25 */    "channel",
/* 26 */    "load",
/* 27 */    "help",
/* 28 */    "?"  };

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] ={
/*                   0  1  2 */
/*  0  INT      */  {0, 2, 2},
/*  1  STR      */  {0, 1, 2},
/*  2  CMD      */  {0, 1, 2},
/*  3  OTHER    */  {0, 1, 2},
/*  4  quit     */  {0, 1, 2},
/*  5     q     */  {0, 1, 2},
/*  6  ping     */  {0, 1, 2},
/*  7  file     */  {0, 1, 2},
/*  8  edit     */  {0, 1, 2},
/*  9  quit     */  {0, 0, 0},
/* 10  cancel   */  {0, 1, 0},
/* 11  name     */  {0, 1, 3},
/* 12  mode     */  {0, 1, 4},
/* 13  zero     */  {0, 1, 0},
/* 14  on       */  {0, 1, 0},
/* 15  off      */  {0, 1, 0},
/* 16  system   */  {0, 1, 2},
/* 17  status   */  {0, 0, 2},
/* 18  time     */  {0, 1, 2},
/* 19  set      */  {0, 1, 2},
/* 20  shutdown */  {0, 1, 2},
/* 21  startup  */  {0, 1, 2},
/* 22  reboot   */  {0, 1, 2},
/* 23  save     */  {0, 1, 2},
/* 24  schedule */  {5, 5, 2},
/* 25  channel  */  {1, 1, 2},
/* 26  load     */  {0, 1, 2},
/* 27  help     */  {0, 1, 2},
/* 28  ?        */  {0, 1, 2}};

/*cmd processor functions */
int c_0(CMD_FSM_CB *); /* nop */
int c_1(CMD_FSM_CB *); /* display all valid commands for the current state */
int c_2(CMD_FSM_CB *); /* ping */
int c_3(CMD_FSM_CB *); /* terminate program */
// int c_4(CMD_FSM_CB *); /*  */
   


/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
/*                STATE 0    1     2 */
/*  0  INT      */  { c_0, c_0, c_0},
/*  1  STR      */  { c_0, c_0, c_0},
/*  2  CMD      */  { c_0, c_0, c_0},
/*  3  OTHER    */  { c_0, c_0, c_0},
/*  4  quit     */  { c_3, c_0, c_0},
/*  5     q     */  { c_3, c_0, c_0},
/*  6  ping     */  { c_2, c_0, c_0},
/*  7  file     */  { c_0, c_0, c_0},
/*  8  edit     */  { c_0, c_0, c_0},
/*  9  quit     */  { c_0, c_0, c_0},
/* 10  cancel   */  { c_0, c_0, c_0},
/* 11  name     */  { c_0, c_0, c_0},
/* 12  mode     */  { c_0, c_0, c_0},
/* 13  zero     */  { c_0, c_0, c_0},
/* 14  on       */  { c_0, c_0, c_0},
/* 15  off      */  { c_0, c_0, c_0},
/* 16  system   */  { c_0, c_0, c_0},
/* 17  status   */  { c_0, c_0, c_0},
/* 18  time     */  { c_0, c_0, c_0},
/* 19  set      */  { c_0, c_0, c_0},
/* 20  shutdown */  { c_0, c_0, c_0},
/* 21  startup  */  { c_0, c_0, c_0},
/* 22  reboot   */  { c_0, c_0, c_0},
/* 23  save     */  { c_0, c_0, c_0},
/* 24  schedule */  { c_0, c_0, c_0},
/* 25  channel  */  { c_0, c_0, c_0},
/* 26  load     */  { c_0, c_0, c_0},
/* 27  help     */  { c_1, c_0, c_0},
/* 28  ?        */  { c_1, c_0, c_0}};

/***************start fsm support functions ********************/
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
    int         i;
    for(i=0;i<sizeof(keyword)/4;i++){
        printf("command = %s\n\r",keyword[i]);
    }
    return 0;
}
/* ping BBB */
int c_2(CMD_FSM_CB *cb)
{
    int         i;
	uint8_t   cmd = PING;
    uint8_t   ret = '\0', *ptr;
    int         s;
    char       *cs = "high here\n\0";

    s  = sizeof(cs);
    ptr = (uint8_t *)&s;
    ret = 99;
    
	printf("  sending ping request to C3 <%u>\r\n",cmd);
	s_wbyte(bbb,&cmd);
    printf("  ping <%u> sent\r\n",cmd);
    s_rbyte(bbb,&ret);
    cmd = 1;
    if(ret == ACK){
        s_wbyte(bbb,&cmd);
        for(i=0;i<4;i++) s_wbyte(bbb,ptr++);
        printf("message size %i\n",s);
        ptr = (uint8_t *)cs;
        for(i=0;i<s;i++) {
            printf("sending <%c>\n",*ptr);
            s_wbyte(bbb,ptr++);
        }
    }
    printf("  BBB acknowledge recieved <%u>\n\r",ret);
    
	return 1;
}
/* terminate program */
int c_3(CMD_FSM_CB *cb)
{
	term(1);
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
    if(cmd_action[cb->token_type][cb->state](cb)==0)   //fire off an fsm action routine
        cb->state = cmd_new_state[cb->token_type][cb->state];         //transition to next state
    else
    {
        // printf("*** error returned from action routine\n");
    }
    return;
}



