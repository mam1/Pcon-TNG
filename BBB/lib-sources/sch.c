
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

/* return a pointer to a schedule rec for a given key, day & channel */
// _S_REC *get_sch_rec(_S_TAB *sch, int day, int channel, int hour, int minute){
//     int             i;
//     int             key;

//     printf(" hour %i, minute %i\n", hour, minute);
//     key = (hour * 60) + minute;
//     for (i = 0; i < sch->schedule[day][channel].rcnt; i++){
//         printf("  key from rec %i test key %i\n", sch->schedule[day][channel].rec[i].key, key); 
//         if(sch->schedule[day][channel].rec[i].key == key){
//             printf("key match\n");
//             return &(sch->schedule[day][channel].rec[i]);
//         }
//     }
//     printf("no key match\n");
//     return NULL;
// }

/* delete a schedule record */
int del_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute){
    // _S_REC               rec;
    int                 i,ii;
    int                 key;

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

/* serch for record in a schedule, return record number of match or -1 if no hit */
int find_tmpl_key(_TMPL_DAT *t, int hour, int minute){
	int 				i, key;

	key = hour * 60 + minute;
	for (i = 0; i < t->rcnt; i++) 
		if(t->rec[0].key == key)
			return i;
	return -1;
}

/* delete a template record */
int del_tmpl_rec(_TMPL_DAT *t, int hour, int minute){
    // _S_REC               rec;
    int                 i,ii;
    int                 key;

    key = hour * 60 + minute;

    if(t->rcnt == 1)
    	if(t->rec[0].key == key){
			t->rcnt = 0;
			return 0;
		}
		
    for (i = 0; i < t->rcnt; i++) {
        if ((t->rec[i].key) == key){
        	// printf("hit\n");
        	for(ii = i;ii<t->rcnt - 1;ii++){
        		// printf("moving\n");
        	 	t->rec[ii] = t->rec[ii+1];
        	}
        	t->rcnt = t->rcnt -1;
        	return 0;
        } 
	}

	return 1;
}

// /* add or change a schedule template record */
// int add_tem_rec2(_TMPL_DAT *t_sch, int hour, int minute, int state, int temp, int humid){
//     int             key;
//     // int          trec;
//     _S_REC          hrec;
//     int             i,ii;

//     key = hour * 60 + minute;
//     hrec.key = key;
//     hrec.state = state;
//     hrec.temp = temp;
//     hrec.humid = humid;

//     for(i=0;i<t_sch->temp_chan_sch.rcnt;i++){
//         if(t_sch->temp_chan_sch.rec[i].key == key){      // record exists, change it
//             t_sch->temp_chan_sch.rec[i] = hrec;
//             return 0;
//         }
//     }
//     if((t_sch->temp_chan_sch.rcnt + 1) > _MAX_SCHEDULE_RECS){    // see if there is room to add another record
//         printf("  too many schedule records\n");
//         return 1;
//     }
//     t_sch->temp_chan_sch.rcnt = t_sch->temp_chan_sch.rcnt + 1;        //add a new record
//     if(t_sch->temp_chan_sch.rcnt == 1){  //first record
//         t_sch->temp_chan_sch.rec[0] = hrec;
//         printf("  first record created, key=%i\n", t_sch->temp_chan_sch.rec[0].key);
//         printf("  rcnt=%i\n", t_sch->temp_chan_sch.rcnt);
//         return 0;
//     }
//     /* search schedule */
//     for (i = 0; i < t_sch->temp_chan_sch.rcnt - 1; i++) {
//         // trec = i;
//         if ((t_sch->temp_chan_sch.rec[i].key) > key) {
//             /* move records down */
//             for (ii = t_sch->temp_chan_sch.rcnt; ii > i; ii--) {
//                 t_sch->temp_chan_sch.rec[ii] = t_sch->temp_chan_sch.rec[ii - 1];
//             }
//             /* insert new record */
//             t_sch->temp_chan_sch.rec[i] = hrec;
//             return 0;
//         }
//         t_sch->temp_chan_sch.rec[t_sch->temp_chan_sch.rcnt - 1] = hrec;
//         return 0;
//     }
//     printf("**** this should not be happening\n\n");
//     return 1;
// }


// /* add or change a schedule record */
// int add_sch_rec2(_S_TAB *sch, int day, int channel, int hour, int minute, int state, int temp, int humid){
//     int             key;
//     // int          trec;
//     _S_REC          hrec;
//     int             i,ii;

//     key = hour * 60 + minute;
//     hrec.key = key;
//     hrec.state = state;
//     hrec.temp = temp;
//     hrec.humid = humid;

//     for(i=0;i<sch->schedule[day][channel].rcnt;i++){
//         if(sch->schedule[day][channel].rec[i].key == key){      // record exists, change it
//             sch->schedule[day][channel].rec[i] = hrec;
//             return 0;
//         }
//     }
//     if((sch->schedule[day][channel].rcnt + 1) > _MAX_SCHEDULE_RECS){    // see if there is room to add another record
//         printf("  too many schedule records\n");
//         return 1;
//     }
//     sch->schedule[day][channel].rcnt = sch->schedule[day][channel].rcnt + 1;        //add a new record
//     if(sch->schedule[day][channel].rcnt == 1){  //first record
//         sch->schedule[day][channel].rec[0] = hrec;
//         printf("  first record created, key=%i\n", sch->schedule[day][channel].rec[0].key);
//         printf("  rcnt=%i\n", sch->schedule[day][channel].rcnt);
//         return 0;
//     }
//     /* search schedule */
//     for (i = 0; i < sch->schedule[day][channel].rcnt - 1; i++) {
//         // trec = i;
//         if ((sch->schedule[day][channel].rec[i].key) > key) {
//             /* move records down */
//             for (ii = sch->schedule[day][channel].rcnt; ii > i; ii--) {
//                 sch->schedule[day][channel].rec[ii] = sch->schedule[day][channel].rec[ii - 1];
//             }
//             /* insert new record */
//             sch->schedule[day][channel].rec[i] = hrec;
//             return 0;
//         }
//         sch->schedule[day][channel].rec[sch->schedule[day][channel].rcnt - 1] = hrec;
//         return 0;
//     }
//     printf("**** this should not be happening\n\n");
//     return 1;
// }

/* add new template record */
int add_tmpl_rec(_TMPL_DAT *t, int hour, int minute, int state, int temp, int humid) {
    int             key;
    _S_REC          hrec;
    int             i,ii;

    /* see if the record exists */
    key = hour * 60 + minute;
    for (i = 0; i < t->rcnt - 1; i++) 
    	if((t->rec[i].key) == key){
			t->rec[i].state = 1;
			return 0;
		}
    if((t->rcnt + 1) > _MAX_SCHEDULE_RECS){    		// see if there is room to add another record
        printf("**** too many schedule records\n");
        return 1;
    }
    t->rcnt += 1;
    hrec.key = key;
    hrec.temp = temp;
    hrec.state = state;
    hrec.humid = humid;   

    if(t->rcnt == 1){  //first record
        t->rec[0] = hrec;
        return 0;
    }
    /* search schedule */
    for (i = 0; i < t->rcnt - 1; i++) 
        if ((t->rec[i].key) > key){
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

/* update state in template record */
// int up_sch_rec_state(_S_TAB *sch, int day, int channel, int hour, int minute, int state){
//     _S_REC              *hrec_ptr, hrec;

//     hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
//     if(hrec_ptr == NULL)
//         return 1;
//     hrec = *hrec_ptr;
//     add_sch_rec2(sch, day, channel, hour, minute, state, hrec.temp, hrec.humid);
//     return 0;
// }

/* set humiduty in schedule record */
// int up_sch_rec_humid(_S_TAB *sch, int day, int channel, int hour, int minute, int humid){
//     _S_REC              *hrec_ptr, hrec;

//     hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
//     if(hrec_ptr == NULL)
//         return 1;
//     hrec = *hrec_ptr;

//     add_sch_rec2(sch, day, channel, hour, minute, hrec.state, hrec.temp, humid);
//     return 0;
// }

/* set temperature in schedule record */
// int up_sch_rec_temp(_S_TAB *sch, int day, int channel, int hour, int minute, int temp){
//     _S_REC              *hrec_ptr, hrec;

//     /* record */
//     hrec_ptr = get_sch_rec(sch, day, channel, hour, minute);
//     printf("get_sch_rec returned %i\n",(int)hrec_ptr ); 
//     if(hrec_ptr == NULL)
//         return 1;
//     hrec = *hrec_ptr;
//     printf("add_sch_rec2 returned %i\n", add_sch_rec2(sch, day, channel, hour, minute, hrec.state, temp, hrec.humid));
//     return 0;
// }

/* convert a key to hous and minutes */
int con_key(int key,int *hour,int *minute){
    *hour = key/60;
    *minute = key%60;
    return key;
}

/* print the schedule for a (day,channel) */
// int dump_schedule(_S_TAB *sch, int day, int channel){
//     int             i,h,m;
//     printf("  dumping schedule for day %i channel %i ", day, channel);
//     printf("    rcnt = %i\n", sch->schedule[day][channel].rcnt);
//     for(i=0;i<sch->schedule[day][channel].rcnt;i++){
//         con_key(sch->schedule[day][channel].rec[i].key,&h,&m);
//         printf("    %02i:%02i - state %i temp %i humid %i\n",
//             h,m,sch->schedule[day][channel].rec[i].state,sch->schedule[day][channel].rec[i].temp, sch->schedule[day][channel].rec[i].humid);
//     }
//     return 0;
// }

/* print a template schedule */
int dump_template(_TMPL_DAT *t_sch){
    int             i,h,m;

    printf("    rcnt = %i\n\r", t_sch->rcnt);
    for(i=0;i<t_sch->rcnt;i++){
        con_key(t_sch->rec[i].key,&h,&m);
        printf("    %02i:%02i - state %i temp %i humid %i\n\r",
            h,m,t_sch->rec[i].state,t_sch->rec[i].temp, t_sch->rec[i].humid);
    }
    return 0;
}

/* append template record list to string */
int load_temps(_TMPL_DAT *t_sch, char *b){
    int             i,h,m;
    char 			tbuff[_PROMPT_BUFFER_SIZE];

    if(t_sch->rcnt < 1){
    	strcat(b, "    no records in schedule");
    	return 0;
    }
    for(i=0;i<t_sch->rcnt;i++){
        con_key(t_sch->rec[i].key,&h,&m);
        sprintf(tbuff,"\r    %02i:%02i - state %i temp %i humid %i\r\n",
        	h,m,t_sch->rec[i].state,t_sch->rec[i].temp, t_sch->rec[i].humid);
        strcat(b, tbuff);
    }
    return 0;
}


// /* print a formated dump of all schedules for a channel */
// void disp_channel_sch(_S_tab *c_dat, channel, char *name, int header)
// {

//     int             i;
//     int             day;
//     char            time_state[9];
//     int             rcnt[_DAYS_PER_WEEK], mrcnt;

//     /* print channel header if requested*/
//     if(header){
//         printf("\r\nchannel %i <%s>\r\n", channel, name);
//         /* print day header */
//         printf("   ");
//         for (day = 0; day < _DAYS_PER_WEEK; day++)
//             printf("%s         ", day_names_short[day]);
//         printf("\n\r");
//     }
//     /* figure the maximum number of transitions per day */
//     mrcnt = 0;
//     for (day = 0; day < _DAYS_PER_WEEK; day++)
//     {

//         schedule[day][channel].rcnt

//         if (rcnt[day] > mrcnt)
//             mrcnt = rcnt[day];        //max number of records for the week
//     }

//     /* Print the daily schedules */
//     for (i = 0; i < mrcnt; i++)
//     {
//         // printf("         ");
//         for (day = 0; day < _DAYS_PER_WEEK; day++)
//         {
//             rec_ptr = get_schedule(sch, day, channel);
//             rec_ptr += (i + 1);
//             if (*get_schedule(sch, day, channel) <= i)
//                 strcpy(time_state, "         ");
//             else
//                 sprintf(time_state, "%02i:%02i %s", get_key((uint32_t)*rec_ptr) / 60, get_key((uint32_t)*rec_ptr) % 60, onoff[get_state((uint32_t)*rec_ptr)]);

//             printf("%s   ", time_state);

//         }
//         printf("\n\r");
//     }
//     printf("\n\r");
//     return;
// }

// /* print a formated dump of schedules for all channels */
// void disp_all_schedules(_S_TAB *sysdat)
// {
//     uint32_t        *rec_ptr;
//     int             i;
//     int             day, channel;
//     char            time_state[9];
//     int             rcnt[_DAYS_PER_WEEK], mrcnt;

//     for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++)
//     {
//         /* print channel header */
//         printf("\r\nchannel %i <%s>\r\n", channel, cb->sdat_ptr->c_data[channel].name);
//         sysdat->schedule[][].

//         /* print day header */
//         printf("   ");
//         for (day = 0; day < _DAYS_PER_WEEK; day++)
//             printf("%s         ", day_names_short[day]);
//         printf("\n\r");

//         /* figure the maximum number of transitions per day */
//         mrcnt = 0;
//         for (day = 0; day < _DAYS_PER_WEEK; day++)
//         {
//             rcnt[day] = *(get_schedule(sch, day, channel));
//             if (rcnt[day] > mrcnt)
//                 mrcnt = rcnt[day];        //max number of records for the week
//         }

//         /* Print the daily schedules */
//         for (i = 0; i < mrcnt; i++)
//         {
//             // printf("         ");
//             for (day = 0; day < _DAYS_PER_WEEK; day++)
//             {
//                 rec_ptr = get_schedule(sch, day, channel);
//                 rec_ptr += (i + 1);
//                 if (*get_schedule(sch, day, channel) <= i)
//                     strcpy(time_state, "         ");
//                 else
//                     sprintf(time_state, "%02i:%02i %s", get_key((uint32_t)*rec_ptr) / 60, get_key((uint32_t)*rec_ptr) % 60, onoff[get_state((uint32_t)*rec_ptr)]);

//                 printf("%s   ", time_state);

//             }
//             printf("\n\r");
//         }
//         printf("\n\r");
//     }

//     return;
// }