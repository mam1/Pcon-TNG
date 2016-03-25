
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


/* add or change a schedule record */
int add_sch_rec(_SCH *sch, int day, int channel, int s_id, int hour, int minute, int state, int temp, int humid){
	int 			key;
	int 			trec;
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
		trec = i;
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

/* convert a key to hous and minutes */
int con_key(int key,int *hour,int *minute){
	*hour = key/60;
	*minute = key%60;
	return key;
}

/* print the schedule for a (day,channel) */
int dump_schedule(_SCH *sch, int day, int channel){
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

	_SCH 		sch, sch2, sch3, sch4;
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
	printf("  enter sensor ID number > ");
	scanf("%i", &sensor_id);
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

	printf("\n  creating schedule record for channel %i, day %i, sensor_id %i, time %02i:%02i, state %i, temp %i humid %i\n\n",
		channel,day, sensor_id, hour, minute, state, temp, humid);

	add_sch_rec(&sch,day,channel,sensor_id,hour,minute,state,temp,humid);
	add_sch_rec(&sch,day,channel,sensor_id,hour+5,minute,state,temp+1,humid);
	add_sch_rec(&sch,day,channel,sensor_id,hour-2,minute,state,temp+2,humid);
	add_sch_rec(&sch,day,channel,sensor_id,hour-7,minute,state,temp+3,humid);

	dump_schedule(&sch,day,channel);
	





	// sch[0] = 3;

	// key[0] = make_key(1, 10);
	// key[1] = make_key(1, 20);
	// key[2] = make_key(1, 30);

	// for(i=0;i<3;i++)
	// 	printf("key %i is %i\n",i, key[i]);

	// s = 1;
	// printf("start loop\n");
	// for(i=0;i<3;i++){
	// 	put_key(&sch[i+1], key[i]); 		// load key into a schedule record
	// 	put_state(&sch[i+1], s);         // load state into a schedule record
	// 	printf("state <%i> loaded into rec with key <%i>\n",s,key[i]);
	// 	if(s == 1)
	// 		s = 0;
	// 	else
	// 		s = 1;
	// }
	// printf("schedule record initialized\ndump schedule:\n");

	// for(i=1;i<4;i++){
	// 	printf("    key = %i, state = %i\n", key[i-1], get_s(sch[i]));
	// }
	// printf("\ntest keys against schedule\n");

	// tkey = make_key(1, 10);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(1, 20);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(1, 30);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(0, 32);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(1, 12);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(1, 25);
	// printf("  test key <%i>  -  ", tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	// tkey = make_key(1, 32);
	// printf("  test key <%i>  -  ",tkey);
	// printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key




	printf("\n **** ending\n");
	return (0);
}

