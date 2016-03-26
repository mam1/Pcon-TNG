
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
// #include "gpio.h"
// #include "led.h"
// #include "schedule.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *c_mode[4] = {"manual", "  time", "   t&s", " cycle"};

/*******************************************/

/* return a pointer to a schedule rec for a given key, day & channel */
_S_REC *get_sch_rec(_S_TAB *sch, int day, int channel, int hour, int minute){
	int 			i;
	int 			key;

	printf(" hour %i, minute %i\n", hour, minute);
	key = (hour * 60) + minute;
	for (i = 0; i < sch->schedule[day][channel].rcnt; i++){
		printf("  key from rec %i test key %i\n", sch->schedule[day][channel].rec[i].key, key); 
		if(sch->schedule[day][channel].rec[i].key == key){
			printf("key match\n");
			return &(sch->schedule[day][channel].rec[i]);
		}
	}
	printf("no key match\n");
	return NULL;
}

/* delete a schedule record */
int del_sch_rec(_S_TAB *sch, int day, int channel, int hour, int minute){
	// _S_REC 				rec;
	int 				i,ii;
	int 				key;

	key = hour * 60 + minute;
	for (i = 0; i < sch->schedule[day][channel].rcnt; i++) 
		if(sch->schedule[day][channel].rec[i].key == key){
			for(ii = i;ii<sch->schedule[day][channel].rcnt - 1;ii++)
				sch->schedule[day][channel].rec[i] = sch->schedule[day][channel].rec[i+1];
			sch->schedule[day][channel].rcnt = sch->schedule[day][channel].rcnt - 1;
			return 0;
		}
	return 1;
}

/* add or change a schedule record */
int add_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute, int state, int temp, int humid){
	int 			key;
	// int 			trec;
	_S_REC 			hrec;
	int 			i,ii;

	key = hour * 60 + minute;
	hrec.key = key;
	hrec.state = state;
	hrec.temp = temp;
	hrec.humid = humid;

	for(i=0;i<sch->schedule[day][channel].rcnt;i++){
		if(sch->schedule[day][channel].rec[i].key == key){  	// record exists, change it
			sch->schedule[day][channel].rec[i] = hrec;
			return 0;
		}
	}
	if((sch->schedule[day][channel].rcnt + 1) > _MAX_SCHEDULE_RECS){	// see if there is room to add another record
		printf("  too many schedule records\n");
		return 1;
	}
	sch->schedule[day][channel].rcnt = sch->schedule[day][channel].rcnt + 1;		//add a new record
	if(sch->schedule[day][channel].rcnt == 1){	//first record
		sch->schedule[day][channel].rec[0] = hrec;
		printf("  first record created, key=%i\n", sch->schedule[day][channel].rec[0].key);
		printf("  rcnt=%i\n", sch->schedule[day][channel].rcnt);
		return 0;
	}
	/* search schedule */
	for (i = 0; i < sch->schedule[day][channel].rcnt - 1; i++) {
		// trec = i;
		if ((sch->schedule[day][channel].rec[i].key) > key) {
			/* move records down */
			for (ii = sch->schedule[day][channel].rcnt; ii > i; ii--) {
				sch->schedule[day][channel].rec[ii] = sch->schedule[day][channel].rec[ii - 1];
			}
			/* insert new record */
			sch->schedule[day][channel].rec[i] = hrec;
			return 0;
		}
		sch->schedule[day][channel].rec[sch->schedule[day][channel].rcnt - 1] = hrec;
		return 0;
	}
	printf("**** this should not be happening\n\n");
	return 1;
}

/* update state in schedule record */
int up_sch_rec_state(_S_TAB *sch, int day, int channel, int hour, int minute, int state){
	_S_REC 				*hrec_ptr, hrec;

	hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
	if(hrec_ptr == NULL)
		return 1;
	hrec = *hrec_ptr;
	add_sch_rec2(sch, day, channel, hour, minute, state, hrec.temp, hrec.humid);
	return 0;
}

/* update humiduty in schedule record */
int up_sch_rec_humid(_S_TAB *sch, int day, int channel, int hour, int minute, int humid){
	_S_REC 				*hrec_ptr, hrec;

	hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
	if(hrec_ptr == NULL)
		return 1;
	hrec = *hrec_ptr;
	add_sch_rec2(sch, day, channel, hour, minute, hrec.state, hrec.temp, humid);
	return 0;
}

/* update temperature in schedule record */
int up_sch_rec_temp(_S_TAB *sch, int day, int channel, int hour, int minute, int temp){
	_S_REC 				*hrec_ptr, hrec;

	/* record */
	hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
	printf("get_sch_rec returned %i\n",(int)hrec_ptr ); 
	if(hrec_ptr == NULL)
		return 1;
	hrec = *hrec_ptr;
	printf("add_sch_rec2 returned %i\n", add_sch_rec2(sch, day, channel, hour, minute, hrec.state, temp, hrec.humid));
	return 0;
}

/* convert a key to hous and minutes */
int con_key(int key,int *hour,int *minute){
	*hour = key/60;
	*minute = key%60;
	return key;
}

/* print the schedule for a (day,channel) */
int dump_schedule(_S_TAB *sch, int day, int channel){
	int 			i,h,m;
	printf("  dumping schedule for day %i channel %i ", day, channel);
	printf("    rcnt = %i\n", sch->schedule[day][channel].rcnt);
	for(i=0;i<sch->schedule[day][channel].rcnt;i++){
		con_key(sch->schedule[day][channel].rec[i].key,&h,&m);
		printf("    %02i:%02i - state %i temp %i humid %i\n",
			h,m,sch->schedule[day][channel].rec[i].state,sch->schedule[day][channel].rec[i].temp, sch->schedule[day][channel].rec[i].humid);
	}
	return 0;
}

int main (void) {

	_S_TAB 		sch;
	// _S_CHN 		hrec;
	int 		day;
	int 		channel;
	int 		hour;
	int 		minute;
	int 		state;
	int 		temp;
	int 		humid;
	int 		sensor_id;


	printf("\n **** starting\n");
	printf("  size of schedule %i bytes\n", sizeof(sch));

	/* create schedule records */
	printf("  enter channel number > ");
	scanf("%i", &channel);
	printf("  enter day number > ");
	scanf("%i", &day);
	printf("  enter hour > ");
	scanf("%i", &hour);
	printf("  enter minute > ");
	scanf("%i", &minute);
	printf("  enter state > ");
	scanf("%i", &state);
	printf("  enter temp > ");
	scanf("%i", &temp);
	printf("  enter humid > ");
	scanf("%i", &humid);

	printf("\n  creating schedule record for channel %i, day %i, time %02i:%02i, state %i, temp %i humid %i\n\n",
		channel,day, hour, minute, state, temp, humid);

	add_sch_rec2(&sch,day,channel,hour,minute,state,temp,humid);
	add_sch_rec2(&sch,day,channel,hour+5,minute,state,temp+1,humid);
	add_sch_rec2(&sch,day,channel,hour-2,minute,state,temp+2,humid);
	add_sch_rec2(&sch,day,channel,hour-7,minute,state,temp+3,humid);
	dump_schedule(&sch,day,channel);

	printf("delete day %i channel %i %02i:%02i\n",day, channel, hour, minute);
	del_sch_rec(&sch,day,channel,hour,minute);
	dump_schedule(&sch,day,channel);

	temp = 99;
	printf("update temp to %i for day %i channel %i %02i:%02i\n",temp, day, channel, hour-2, minute);
	up_sch_rec_temp(&sch, day, channel, hour-2, minute, temp);
	dump_schedule(&sch,day,channel);

	printf("\n **** ending\n");
	return (0);
}

