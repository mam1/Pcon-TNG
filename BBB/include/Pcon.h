/**************************************************/
/*  Pcon.h 		                                  */
/*  header file for Pcon.c                        */
/*                                                */
/**************************************************/
#ifndef PCON_H_
#define PCON_H_


/* Pcon version info */
#define _MAJOR_VERSION_Pcon    	14
#define _MINOR_VERSION_Pcon   	0
#define _MINOR_REVISION_Pcon   	1

// #define _TRACE 				// turn on trace
#include "shared.h"

/* fuctions */
void prompt(int);
void del_prompt(int);
int term(int);
void term1(void);
void disp_sys(void);
char *cmd_def(int token_number, int state);
void arrow_reprompt(void);

#endif
