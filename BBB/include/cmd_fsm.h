/**************************************************/
/*  cmd_fsm.h version 0.0                         */
/*  header file for cmd_fsm.c                     */
/*                                                */
/**************************************************/
#ifndef _CMD_FSM_H_
#define _CMD_FSM_H_

/* fuctions */
void cmd_fsm(_CMD_FSM_CB *);
void cmd_fsm_reset(_CMD_FSM_CB *);
int cmd_type(char *);
char *build_prompt(char *, int);
void reset_active(void);
void reset_edit(void);
// void save_system_data(char *, SYS_DAT *);
// void load_system_data(char *, SYS_DAT *);

#endif
