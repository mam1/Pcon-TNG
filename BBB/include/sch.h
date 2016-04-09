#ifndef _SCH_H_
#define _SCH_H_


/* return a pointer to a schedule rec for a given key, day & channel */
_S_REC *get_sch_rec(_S_TAB *sch, int day, int channel, int hour, int minute);

/* delete a schedule record */
int del_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute);

/* add or change a schedule record */
int add_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute, int state, int temp, int humid);

/* update state in schedule record */
int up_sch_rec_state(_S_TAB *sch, int day, int channel, int hour, int minute, int state);

/* update humiduty in schedule record */
int up_sch_rec_humid(_S_TAB *sch, int day, int channel, int hour, int minute, int humid);

/* update temperature in schedule record */
int up_sch_rec_temp(_S_TAB *sch, int day, int channel, int hour, int minute, int temp);

/* convert a key to hous and minutes */
int con_key(int key,int *hour,int *minute);

/* print the schedule for a (day,channel) */
int dump_schedule(_S_TAB *sch, int day, int channel);

/* print a formated dump of schedules for all channels */
void disp_all_schedules(_S_TAB *sch);

/* add or change a schedule template record */
int add_tem_rec2(_TMPL_DAT *c_sch, int hour, int minute, int state, int temp, int humid);

/* print a template schedule */
int dump_template(_TMPL_DAT *t_sch);

/* load template dump into bufer */
int load_temps(_TMPL_DAT *t_sch, char *b);

/* add new template record */
int add_tmpl_rec(_TMPL_DAT *t, int hour, int minute, int state, int temp, int humid);

/* print a formated dump of all days for a channel */
// void disp_channel_sch(_S_CHAN *c_dat,int header);

#endif /* _SCH_H_ */