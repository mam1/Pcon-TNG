/************************************************************************/
/*  schedule.c                                                          */
/*      Functions which support loading and saving schedules.           */
/*      Each (day of the week, channel)has a unique schedule.           */
/*      A schedule is an array of unsigned int where the first int      */
/*      is the number of records in the schedule the following int      */
/*      indicate a time and state transition for each channel.          */
/*                                                                      */
/*      A schedule record is a unsigned 32 bit value.  The high bit     */
/*      indicated the state (on or off) the lower bits contain the      */
/*                                                                      */
/*      record key (minutes from 00:00)                                 */
/************************************************************************/

/***************************** includes *********************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>     //uint_8, uint_16, uint_32, etc.
#include <unistd.h>
#include "Pcon.h"
#include "schedule.h"
#include "bitlit.h"
#include "typedefs.h"

/****************************** externals *******************************/

/******************** global code to text conversion ********************/
extern char *day_names_long[7];
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];

/************************** edit state variable *************************/
extern int     edit_channel, edit_day, edit_hour, edit_minute, edit_key;

/***************************** globals **********************************/
uint32_t       state_mask = B32(10000000, 00000000, 00000000, 00000000);
uint32_t       key_mask   = B32(01111111, 11111111, 11111111, 11111111);

/*******************************  functions ******************************/


/* create key */
int make_key(int hour, int minute) {

	return (hour * 60) + minute;
}

/* operate on a single record */
int get_key(uint32_t b)                         // extract key from a schedule record
{
	uint32_t     k;
	k = (int)(b & key_mask);
	return (int)k;
}

int get_s(uint32_t b)                           // extract state from a schedule record
{
	if (b & state_mask)
		return 1;
	return 0;
}

void put_key(volatile uint32_t *value, int key) // load key into a schedule record
{
	int         hold_state;
	uint32_t    t;

	hold_state = get_s(*value);
	t = (uint32_t)key;
	if (*value & state_mask) t |= state_mask;
	*value = key;
	put_state(value, hold_state);
	return;
}

void put_state(volatile uint32_t *b, int s)     // load state into a schedule record
{
	// printf("setting state to %i\n",s);
	if (s) *b |= state_mask;
	else  *b &= ~state_mask;
	return;
}

int add_sch_rec(uint32_t *sch, int k, int s)    // add or change a schedule record */
{
	uint32_t       *end, *r;

	/* schedule has no records - insert one */
	if ((int)*sch == 0)
	{
		*sch++ = 1;
		put_state(sch, s);
		put_key(sch, k);
		return 0;
	}
	/* see if there is room to add another record */
	if ((int)*sch >= _MAX_SCHEDULE_RECS)
	{
		printf("*** too many schedule records\n");
		return 1;
	}
	/* if record exists change it */
	r = find_schedule_record(sch, k);
	if (r)
	{
		put_state(r, s);
		return 0;
	}
	/* insert new record in ordered list */
	*sch += 1;                //increase record count
	end = (sch + *sch) - 1;   //set pointer to end of the list
	sch++;
	printf("\n\n");
	while (sch <= end)
	{
		if (k < get_key(*sch))
			break;
		sch++;
	}
	while (end >= sch)
		*(end + 1) = *(end--);
	put_state(sch, s);
	put_key(sch, k);
	return 0;
}

int del_sch_rec(uint32_t *sch, int k)           // delete a schedule record with matching key
{
	uint32_t            *rsize;
	int              i, hit;

	if (*sch == 0)
		return 0;
	if (*sch == 1)
	{
		*sch = 0;
		return 0;
	}

	hit = 0;
	rsize = sch++;

	for (i = 0; i < *rsize; i++)
	{
		if ((k == get_key(*sch)) || (hit == 1))
		{
			hit = 1;
			*sch = *(sch + 1);
		}
		sch++;
	}

	if (hit)
	{
		*rsize -= 1;
		return 0;
	}
	return 1;
}

uint32_t *find_schedule_record(uint32_t *sch, int k) // search schedule for record with key match, return pointer to record or NULL
{
	int                              i, rsize;

	rsize = (int) * sch++;
	for (i = 0; i < rsize; i++)
	{
		if (k == get_key(*sch))
			return sch;
		sch++;
	}
	return NULL;
}

void disp_all_schedules(CMD_FSM_CB *cb, uint32_t *sch)
{
	uint32_t        *rec_ptr;
	int             i;
	int             day, channel;
	char            time_state[9];
	int             rcnt[_DAYS_PER_WEEK], mrcnt;


	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++)
	{
		/* print channel header */
		printf("\r\nchannel %i <%s>\r\n", channel, cb->sdat_ptr->c_data[channel].name);

		/* print day header */
		printf("   ");
		for (day = 0; day < _DAYS_PER_WEEK; day++)
			printf("%s         ", day_names_short[day]);
		printf("\n\r");

		/* figure the maximum number of transitions per day */
		mrcnt = 0;
		for (day = 0; day < _DAYS_PER_WEEK; day++)
		{
			rcnt[day] = *(get_schedule(sch, day, channel));
			if (rcnt[day] > mrcnt)
				mrcnt = rcnt[day];        //max number of records for the week
		}

		/* Print the daily schedules */
		for (i = 0; i < mrcnt; i++)
		{
			// printf("         ");
			for (day = 0; day < _DAYS_PER_WEEK; day++)
			{
				rec_ptr = get_schedule(sch, day, channel);
				rec_ptr += (i + 1);
				if (*get_schedule(sch, day, channel) <= i)
					strcpy(time_state, "         ");
				else
					sprintf(time_state, "%02i:%02i %s", get_key((uint32_t)*rec_ptr) / 60, get_key((uint32_t)*rec_ptr) % 60, onoff[get_s((uint32_t)*rec_ptr)]);

				printf("%s   ", time_state);

			}
			printf("\n\r");
		}
		printf("\n\r");
	}

	return;
}


void load_schedule(uint32_t *sch, uint32_t *template, int day, int channel)   // load schedule buffer w
{
	int         i;
	uint32_t    *start_schedule;
	start_schedule = get_schedule(sch, day, channel);
	for (i = 0; i < _SCHEDULE_SIZE; i++)
		*start_schedule++ = *template++;

	return;
}

uint32_t *get_schedule(uint32_t *sbuf,int d,int c)  // return pointer to  a schedule
{
   SCH             *sch_ptr;
   DAY             *day_ptr;

   day_ptr = (DAY *)sbuf;      //set day pointer to the start of the schedule buffer
   day_ptr += d;             //move day pointer to the start of the requested day
   sch_ptr = (SCH *)day_ptr;   //set channel pointer to the start of the requested day
   sch_ptr += c;               //move channel pointer to the requested channel

   return (uint32_t *)sch_ptr;
}

int test_sch(uint32_t *r, int k) //return state for key
{
	uint32_t              *last_record, *first_record;
	// uint32_t				rcnt;

	if (*r == 0) return 0;     	// test for no schedule records

	last_record =  r + *r;  	// add number of records to first record pointer to set pointer to last record
	r++;						// move pointer to first schedule record
	first_record = r;
	while (r <= last_record)   	// search schedule for corresponding to key
	{
		if (get_key(*r) == k)
			return get_s(*r);
		if ((get_key(*r) > k))
			if(r == first_record)
				return get_s(*last_record);
			else
				return get_s(*(--r));
		r++;
	}
	return get_s(*last_record);
}

void disp_sch(uint32_t *sch)
{
	uint32_t        *rec_ptr;
	int             i;
	int             day, channel;
	char            time_state[9];
	int             rcnt[_DAYS_PER_WEEK], mrcnt;


	for (channel = 0; channel < _NUMBER_OF_CHANNELS; channel++)
	{


		/* print day header */
		printf("   ");
		for (day = 0; day < _DAYS_PER_WEEK; day++)
			printf("%s         ", day_names_short[day]);
		printf("\n\r");

		/* figure the maximum number of transitions per day */
		mrcnt = 0;
		for (day = 0; day < _DAYS_PER_WEEK; day++)
		{
			rcnt[day] = *(get_schedule(sch, day, channel));
			if (rcnt[day] > mrcnt)
				mrcnt = rcnt[day];        //max number of records for the week
		}

		/* Print the daily schedules */
		for (i = 0; i < mrcnt; i++)
		{
			// printf("         ");
			for (day = 0; day < _DAYS_PER_WEEK; day++)
			{
				rec_ptr = get_schedule(sch, day, channel);
				rec_ptr += (i + 1);
				if (*get_schedule(sch, day, channel) <= i)
					strcpy(time_state, "         ");
				else
					sprintf(time_state, "%02i:%02i %s", get_key((uint32_t)*rec_ptr) / 60, get_key((uint32_t)*rec_ptr) % 60, onoff[get_s((uint32_t)*rec_ptr)]);

				printf("%s   ", time_state);

			}
			printf("\n\r");
		}
		printf("\n\r");
	}

	return;
}


// uint32_t *get_schedule(uint32_t *sch, int day, int channel) { // return pointer to a schedule for a (day,channel)

// 	uint32_t    *start_schedule;

// 	start_schedule = sch;
// 	while (day > 0) {       //move day pointer to the requested day
// 		start_schedule += _BYTES_PER_DAY;
// 		day--;
// 	}
// 	while (channel > 0) {   //move channel pointer to the requested channel
// 		start_schedule += _SCHEDULE_SIZE;
// 		channel--;
// 	}

// 	return (uint32_t *)start_schedule;
// 	// return sch;
// }

