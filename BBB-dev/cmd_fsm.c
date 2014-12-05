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


/*********************** externals **************************/
 extern int          cmd_state,char_state;
 extern char         input_buffer[_INPUT_BUFFER],*input_buffer_ptr;
 extern char         c_name[_CHANNEL_NAME_SIZE][_NUMBER_OF_CHANNELS];
 extern int 		 exit_flag;		//exit man loop if TRUE
 extern int			 trace_flag;
 extern int 		 bbb;				//UART1 file dscriptor

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
int c_0(int,int *,char *); /* nop */
int c_1(int,int *,char *); /* display all valid commands for the current state */
int c_2(int,int *,char *); /* ping */
int c_3(int,int *,char *); /* terminate program */


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
/* 27  help     */  { c_0, c_0, c_0},
/* 28  ?        */  { c_0, c_0, c_0}};

/***************start fsm support functions ********************/




/**************** command fsm action routines ******************/
/* do nothing */
int c_0(int tt, int *n, char *s)
{
    return 0;
}
/* display all valid commands for the current state */
int c_1(int tt, int *n, char *s)
{

    return 0;
}
/* ping BBB */
int c_2(int tt, int *n, char *s)
{
	char	cmd = 'p';
//	printf("  sending ping request to BBB <%u>\n\r",cmd);
	write(bbb,&cmd,1);
	printf("  BBB acknowledge recieved\n");
	return 0;
}
/* terminate program */
int c_3(int tt, int *n, char *s)
{
	term(1);
    return 0;
}

/*****************************************************/
/*********  command parser state machine end  ********/
/*****************************************************/
/* cycle state machine */
void cmd_fsm(char *token,int *state)
{
    static int         tt, num,*n_ptr;
    static char        *s_ptr;

    tt = cmd_type(token);
#ifdef _TRACE
		    sprintf(trace_buf, "cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",token,tt, *state);
			strace(_TRACE_FILE_NAME,trace_buf,trace_flag);
#endif
//    printf("cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",token,tt, *state);
    if((tt==1)||(tt==2))
    {
        n_ptr = NULL;
        s_ptr = token;
    }
    else if(tt==0)
    {
        sscanf(token,"%u",&num);
        n_ptr = &num;
        s_ptr = NULL;
    }
    else
    {
        num = tt;
        n_ptr = &num;
        s_ptr = NULL;
    }
//    printf("call cmd_action[%i][%i](<%i>,<%i>,<%s>)\n",tt,*state,tt,*n_ptr,s_ptr);
    if(cmd_action[tt][*state](tt,n_ptr,s_ptr)==0)   //fire off an fsm action routine
        *state = cmd_new_state[tt][*state];         //transition to next state
    else
    {
        // printf("*** error returned from action routine\n");
    }
    return;
}


/*********************** functions **************************/
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
int cmd_type(char *c)
{
    int     i;
    /*test for an empty command que */
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
        if(strlen(c) == strlen(keyword[i]))
            if(strncmp(c,keyword[i],strlen(c))==0)
                return i;
    }
    /* unrecognized token */
    return 2;
}
