/**************************************************/
/*  cmd_fsm.h version 0.0                         */
/*  header file for cmd_fsm.c                     */
/*                                                */
/**************************************************/
#ifndef CMD_FSM_H_
#define CMD_FSM_H_

/* fuctions */
void cmd_fsm(CMD_FSM_CB *);
void cmd_fsm_reset(CMD_FSM_CB *);
int cmd_type(char *);
char *build_prompt(char *, int);
void reset_active(void);
void reset_edit(void);
void save_channel_data(char *, SYS_DAT *);
void load_channel_data(char *, SYS_DAT *);

#endif
