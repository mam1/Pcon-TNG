#ifndef SCHEDULE_H_
#define SCHEDULE_H_
#include "typedefs.h"

/* routines that work with a pointer to the full schedule data structure */
 void disp_all_schedules(uint32_t *);		//(channel data,schedule data)
 														//dump the entrire schedule data structure

 void load_schedule(uint32_t *,uint32_t *, int, int);  	//(schedule data, template, day, channel) 
 														//load a schedule template into the schedule data structre
 
 uint32_t *get_schedule(uint32_t *,int,int); 			//(schedule data,day,channel) 
 														//return pointer to a schedule

/* routines to work with individual schedule records */
 int  get_key(uint32_t);          			//extract key (lower 31 bits) from a uint32_t
 int  get_s(uint32_t);        				//extract state (high bit) from a uint32_t
 void put_key(volatile uint32_t *,int);    	//load key into lower 31 bits of the  uint31_t at the address specified
 void put_state(volatile uint32_t *,int);  	//load state into high bit of the uint31_t at the address specified
 uint32_t *find_schedule_record(uint32_t *,int);
 int add_sch_rec(uint32_t *, int, int);
 int del_sch_rec(uint32_t *, int); 
 int make_key(int hour, int minute);


#endif



