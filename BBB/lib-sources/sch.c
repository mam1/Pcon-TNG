
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "shared.h"
#include "bitlit.h"
#include "PCF8563.h"
#include "sys_dat.h"

/* code to text conversion */
extern char *day_names_long[7];
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];
extern char *c_mode[4];

// /* delete a schedule record */
// int del_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute) {
// 	// _S_REC               rec;
// 	int                 i, ii;
// 	int                 key;

// 	key = hour * 60 + minute;
// 	for (i = 0; i < sch->sch[day][channel].rcnt; i++)
// 		if (sch->sch[day][channel].rec[i].key == key) {
// 			for (ii = i; ii < sch->sch[day][channel].rcnt - 1; ii++)
// 				sch->sch[day][channel].rec[i] = sch->sch[day][channel].rec[i + 1];
// 			sch->sch[day][channel].rcnt = sch->sch[day][channel].rcnt - 1;
// 			return 0;
// 		}
// 	return 1;
// }

/* serch for record key match in a schedule, return record number of match or -1 if no hit */
int find_tmpl_key(_TMPL_DAT *t, int hour, int minute) {
	int 				i, key;

	key = hour * 60 + minute;
	for (i = 0; i < t->rcnt; i++)
		if (t->rec[0].key == key)
			return i;
	return -1;
}

/* delete a template record */
int del_tmpl_rec(_TMPL_DAT *t, int hour, int minute) {
	// _S_REC               rec;
	int                 i, ii;
	int                 key;

	key = hour * 60 + minute;

	if (t->rcnt == 1)
		if (t->rec[0].key == key) {
			t->rcnt = 0;
			return 0;
		}

	for (i = 0; i < t->rcnt; i++) {
		if ((t->rec[i].key) == key) {
			// printf("hit\n");
			for (ii = i; ii < t->rcnt - 1; ii++) {
				// printf("moving\n");
				t->rec[ii] = t->rec[ii + 1];
			}
			t->rcnt = t->rcnt - 1;
			return 0;
		}
	}
	return 1;
}

/* add new template record */
int add_tmpl_rec(_TMPL_DAT *t, int hour, int minute, int state, int temp, int humid) {
	int             key;
	_S_REC          hrec;
	int             i, ii;

	/* see if the record exists */
	key = hour * 60 + minute;
	for (i = 0; i < t->rcnt; i++)
		if ((t->rec[i].key) == key) {
			t->rec[i].state = state;
			t->rec[i].temp = temp;
			t->rec[i].humid = humid;
			return 0;
		}
	if ((t->rcnt + 1) > _MAX_SCHEDULE_RECS) {    		// see if there is room to add another record
		printf("**** too many schedule records\n");
		return 1;
	}
	t->rcnt += 1;
	hrec.key = key;
	hrec.temp = temp;
	hrec.state = state;
	hrec.humid = humid;

	if (t->rcnt == 1) { //first record
		t->rec[0] = hrec;
		return 0;
	}
	/* search schedule */
	for (i = 0; i < t->rcnt - 1; i++)
		if ((t->rec[i].key) > key) {
			/* move records down */
			for (ii = t->rcnt; ii > i; ii--)
				t->rec[ii] = t->rec[ii - 1];
			/* insert new record */
			t->rec[i] = hrec;
			return 0;
		}
	t->rec[t->rcnt - 1] = hrec;
	return 0;
}

/* convert a key to hous and minutes */
int con_key(int key, int *hour, int *minute) {
	*hour = key / 60;
	*minute = key % 60;
	return key;
}

/* given a key and schedsule return the state new state based on time of day */
int test_sch_time(int key, _TMPL_DAT *t) {
	int 			state;
	int 			i;

	if(t->rcnt == 0)
		return 0;

	if(t->rcnt == 1)
		return t->rec[0].state;

	for (i = 0; i < t->rcnt; i++){
		if(t->rec[i].key == key)
			return t->rec[i].state;

		if((t->rec[i].key) > key)
			if(i>0)
				return t->rec[i-1].state;
			else
				return t->rec[t->rcnt].state;
	}

	return t->rec[0].state;
	
}


/* print a template schedule */
int dump_template(_TMPL_DAT *t_sch) {
	int             i, h, m;

	printf("    rcnt = %i\n\r", t_sch->rcnt);
	for (i = 0; i < t_sch->rcnt; i++) {
		con_key(t_sch->rec[i].key, &h, &m);
		printf("    %02i:%02i - state %i temp %i humid %i\n\r",
		       h, m, t_sch->rec[i].state, t_sch->rec[i].temp, t_sch->rec[i].humid);
	}
	return 0;
}

/* print a one line template schedule */
int list_template(_TMPL_DAT *t_sch) {
	int             i, h, m;

	// printf("    rcnt = %i\n\r", t_sch->rcnt);
	for (i = 0; i < t_sch->rcnt; i++) {
		con_key(t_sch->rec[i].key, &h, &m);
		printf(" %02i:%02i-%i,%i,%i ",
		       h, m, t_sch->rec[i].state, t_sch->rec[i].temp, t_sch->rec[i].humid);
	}
	printf(" %s ", t_sch->name);
	return 0;
}


/* append template record list to string */
int load_temps(_TMPL_DAT *t_sch, char *b) {
	int             i, h, m;
	char 			tbuff[_PROMPT_BUFFER_SIZE];

	if (t_sch->rcnt < 1) {
		strcat(b, "    no records in schedule");
		return 0;
	}
	for (i = 0; i < t_sch->rcnt; i++) {
		con_key(t_sch->rec[i].key, &h, &m);
		sprintf(tbuff, "\r    %02i:%02i - state %i temp %i humid %i\r\n",
		        h, m, t_sch->rec[i].state, t_sch->rec[i].temp, t_sch->rec[i].humid);
		strcat(b, tbuff);
	}
	return 0;
}

/* print a formated dump of schedules for each channel and day */
void sch_print(_CMD_FSM_CB *cb, _S_TAB *s) {
	// uint32_t        *rec_ptr;
	int             i;
	int             day, channel;
	int 			hour, minute;
	char            time_state[9];
	int             mrcnt;

	for (channel=0; channel < _NUMBER_OF_CHANNELS; channel++)
	{
		/* print channel header */
		printf("\r\nchannel %i <%s>\r\n", channel, cb->sys_ptr->c_data[channel].name);

		/* print day header */
		printf("   ");
		for (day = 0; day < _DAYS_PER_WEEK; day++)
			printf("%s         ", day_names_short[day]);
		printf("\n\r");

		/* figure the maximum number of transitions per day */
		mrcnt = 0;
		for (day=0; day < _DAYS_PER_WEEK; day++)
		{

			if (s->sch[day][channel].rcnt > mrcnt)
				mrcnt = s->sch[day][channel].rcnt;        //max number of records for the week
		}
		// printf(" max records  %i\n\r",mrcnt);

		/* Print the daily schedules */
		for (i = 0; i < mrcnt; i++) {
			for (day = 0; day < _DAYS_PER_WEEK; day++) {
				// printf("records in schedule %i\n\r",cb->sys_ptr->sys_sch.sch[day][channel].rcnt);
				if (s->sch[day][channel].rcnt <= i)
					strcpy(time_state, "         ");
				else {
					con_key(s->sch[day][channel].rec[i].key, &hour, &minute);
					sprintf(time_state, "%02i:%02i %s", hour, minute, onoff[s->sch[day][channel].rec[i].state]);
				}
				printf("%s   ", time_state);
			}
			printf("\n\r");
		}
		printf("\n\r");
	}
	return;
}

