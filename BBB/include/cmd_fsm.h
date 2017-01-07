/**************************************************/
/*  cmd_fsm.h version 0.0                         */
/*  header file for cmd_fsm.c                     */
/*                                                */
/**************************************************/
#ifndef _CMD_FSM_H_
#define _CMD_FSM_H_

/* cmd_fsm.c version */
#define _MAJOR_VERSION_cmd_fsm		10
#define _MINOR_VERSION_cmd_fsm		4
#define _MINOR_REVISION_cmd_fsm		0

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
