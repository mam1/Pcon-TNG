
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
extern char *mode[4];


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
	// printf("********* rcnt on entery %i \r\n", t->rcnt);
	for (i = 0; i < t->rcnt; i++)
		if ((t->rec[i].key) == key) {
			t->rec[i].state = state;
			t->rec[i].temp = temp;
			t->rec[i].humid = humid;
			return 0;
		}

	// printf("********* rcnt on entery %i \r\n", t->rcnt);
	/* see if there is room to add another record */	
	if ((t->rcnt + 1) > _MAX_SCHEDULE_RECS) {    		
		printf("**** too many schedule records\n");
		return 1;
	}

	// printf("********* rcnt on entery 1 %i \r\n", t->rcnt);
	hrec.key = key;
	hrec.temp = temp;
	hrec.state = state;
	hrec.humid = humid;

	// printf("********* rcnt on entery 2 %i \r\n", t->rcnt);
	if (t->rcnt == 0) { // first record
		// printf("*********** first record\n");
		t->rec[0] = hrec;
		t->rcnt += 1;
		return 0;
	}

	// printf("11111111 rcnt before the test %i \r\n", t->rcnt);
	// if (t->rcnt == 1) { // second record
	// 	printf("*********** second record key %i, insert key %i\n",t->rec[0].key, key);
	// 	if(t->rec[0].key > key){
	// 		t->rec[1] = t->rec[0];
	// 		t->rec[0] = hrec;
	// 	}
	// 	else
	// 		t->rec[1] = hrec;

	// 	t->rcnt += 1;
	// 	return 0;
	// }

	// printf("********* rcnt before the search %i \r\n", t->rcnt);
	/* search schedule */
	for (i = 0; i < t->rcnt; i++)
		if ((t->rec[i].key) > key) {
			/* move records up */
			for (ii = t->rcnt; ii >= i; ii--)
				t->rec[ii] = t->rec[ii - 1];
			/* insert new record */
			// printf("********* i = %i\n\r\n", i );
			t->rec[i] = hrec;
			t->rcnt += 1;
			return 0;
		}

	// printf("********* after search t->rcnt = %i\n\r\n", t->rcnt );
	t->rcnt += 1;
	t->rec[t->rcnt - 1] = hrec;
	return 0;
}

/* convert a key to hous and minutes */
int con_key(int key, int *hour, int *minute) {
	// printf("***************** con_key called\r\n");
	*hour = key / 60;
	*minute = key % 60;
	return key;
}

/* given a key and schedule return the state based on time of day */
int test_sch_time(int key, _TMPL_DAT *t) {
	// int 			state;
	int 			i;

	if(t->rcnt == 0)
		return 0;
	if(t->rcnt == 1)
		return t->rec[0].state;
	for (i = 0; i < t->rcnt; i++){
		if(t->rec[i].key == key)
			return t->rec[i].state;

		if((t->rec[i].key) > key){
			if(i>0)
				return t->rec[i-1].state;
		}
		return t->rec[t->rcnt].state;
	}
	return t->rec[i-1].state;
}

/* given a key and schedule return state based on time of day and value of a sensor */
int test_sch_sensor(int key, _TMPL_DAT *t, int sensor) {
	int 			state;
	// int 			delta, h_limit, l_limit, i;

	int 			i;

	if(t->rcnt == 0)
		return 0;
	if(t->rcnt == 1)	
		return slabcon(t->rec[0].temp, sensor);
	for (i = 0; i < t->rcnt; i++){
		if(t->rec[i].key == key)
			return slabcon(t->rec[i].temp, sensor);

		if((t->rec[i].key) > key){
			if(i>0)

				return slabcon(t->rec[i-1].temp, sensor);
		}

		return slabcon(t->rec[t->rcnt].temp, sensor);
	}

	return slabcon(t->rec[i-1].temp, sensor);
}

/* given a key and schedule return the target temperature */
int get_tar_temp(int key, _TMPL_DAT *t) {
	int 			state;
	// int 			delta, h_limit, l_limit, i;

	int 			i;

	if(t->rcnt == 0)
		return 0;
	if(t->rcnt == 1)	
		return t->rec[0].temp;
	for (i = 0; i < t->rcnt; i++){
		if(t->rec[i].key == key)
			return t->rec[i].temp;

		if((t->rec[i].key) > key){
			if(i>0)

				return t->rec[i-1].temp;
		}

		return t->rec[t->rcnt].temp;
	}

	return t->rec[i-1].temp;
}

	// printf("schedule %i\n\r\n",t->rec[0].temp);
	// printf("sensor %i\n\r\n",sensor);
	// delta = t->rec[0].temp - sensor;
	// printf("delta %i\n\r\n",delta);

	// if(t->rcnt == 0)
	// 	return 0;

	// if(t->rcnt == 1){

	// 	l_limit = 10;

	// 	if(delta < 0)
	// 		return 0;

	// 	if(delta > l_limit)
	// 		return 1;

	// 	return 0;
	// }

	// for (i = 0; i < t->rcnt; i++){
	// 	if(t->rec[i].key == key)
	// 		return t->rec[i].state;

	// 	if((t->rec[i].key) > key)
	// 		if(i>0)
	// 			return t->rec[i-1].state;
	// 		else
	// 			return t->rec[t->rcnt].state;
	// }

	// return t->rec[i-1].state;
	

	// return 0;
// }
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
	static char 			tbuff[_PROMPT_BUFFER_SIZE];

	tbuff[0] = '\0';
	// printf(" ^^^^^^ tbuff <%s>\r\n", tbuff);
	// printf("************** load temps called, record count %i\r\n", t_sch->rcnt);
	if (t_sch->rcnt < 1) {
		strcat(b, "    no records in schedule");
		return 0;
	}
	// printf("************** record 2 count %i\r\n", t_sch->rcnt);
	if(t_sch->rcnt > _MAX_SCHEDULE_RECS){
		printf("************* your screwed record count trashed\n\r\n aborting\r\n");
		term1();
	}

	for (i = 0; i < t_sch->rcnt; i++) {
		// printf(" @@ loop i %i\r\n", i);
		// printf("           record count %i\r\n", t_sch->rcnt);
		con_key(t_sch->rec[i].key, &h, &m);
		// printf("           record count a %i\r\n", t_sch->rcnt);
		// printf("           tbuff ((%s))\r\n",tbuff);
		sprintf(tbuff, "\r    %02i:%02i - state %i temp %i humid %i\r\n",
		        h, m, t_sch->rec[i].state, t_sch->rec[i].temp, t_sch->rec[i].humid);
		// printf("           record count %i\r\n", t_sch->rcnt);

		// printf("\n\r      tbuff ((%s))\r\n", tbuff);
		// printf("\n\r      b ((%s))\n\r", b);
		strcat(b, tbuff);
		// printf("           record count end of loop  %i\r\n", t_sch->rcnt);
	}

	// printf("************** load temps returning, record count %i\r\n", t_sch->rcnt);
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

					switch(cb->sys_ptr->c_data[channel].mode){ //Control mode: 0-manual, 1-time, 2-time & sensor, 3-cycle
						case 0:
							sprintf(time_state, "%02i:%02i %s", hour, minute, onoff[s->sch[day][channel].rec[i].state]);
							break;
						case 1:
							break;
							sprintf(time_state, "%02i:%02i %s", hour, minute, onoff[s->sch[day][channel].rec[i].state]);
						case 2:
							sprintf(time_state, "%02i:%02i %i", hour, minute, s->sch[day][channel].rec[i].temp);
							break;
						case 3:
							printf("**** error  bad chnannel mode <%i>\r\n", cb->sys_ptr->c_data[channel].mode);
							break;
						default:
							printf("**** error  bad chnannel mode <%i>\r\n", cb->sys_ptr->c_data[channel].mode);
					}
				}
				printf("%s   ", time_state);
			}
			printf("\n\r");
		}
		printf("\n\r");
	}
	return;
}

