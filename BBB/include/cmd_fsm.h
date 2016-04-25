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
void build_prompt(_CMD_FSM_CB * cb);
void reset_active(void);
void reset_edit(void);

/* return token type; -1 -INT, -2 -QUOTE, -3 -unrecognized, -4 -NULL, command number (0 - xx) */
int token_type(char *c);

// void save_system_data(char *, SYS_DAT *);
// void load_system_data(char *, SYS_DAT *);

#endif
