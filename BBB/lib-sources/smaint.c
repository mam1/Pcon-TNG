/*****************************************************************/
/*  smaint.c -                                                   */
/*  routines to maintain the sensor list                         */
/*                                                               */
/*                                                               */
/*****************************************************************/

#include <unistd.h>		//sleep
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h>		//isdigit, isalnum, tolower
#include <stdbool.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include "Pcon.h"
#include "Dcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"
#include "PCF8563.h"
#include "list_maint.h"
#include "ipc.h"
#include "sys_dat.h"
#include "sch.h"
#include "smaint.h"

extern int 				semid;
extern unsigned short 	semval;
extern struct sembuf	sb;

int s_load(int id, _CMD_FSM_CB *cb)
{
	int 		i;

	for ( i = 0; i < _NUMBER_OF_SENSORS; i++)
		if (cb->ipc_ptr->s_dat[i].sensor_id == id)
		{
			cb->w_sen_dat.sensor_id = id;
			strcpy(cb->w_sen_dat.name, cb->ipc_ptr->s_dat[i].name);
			strcpy(cb->w_sen_dat.description, cb->ipc_ptr->s_dat[i].description);
			return id;
		}
	
	cb->w_sen_dat.sensor_id = id;
	strcpy(cb->w_sen_dat.name, "");
	strcpy(cb->w_sen_dat.description, "");

	return id;
}

int s_sort(_SEN_DAT f[])
{
	_SEN_DAT 		source[_NUMBER_OF_SENSORS];
	_SEN_DAT 		destination[_NUMBER_OF_SENSORS];
	int 			source_deleted[_NUMBER_OF_SENSORS];
	int 			i,ii;
	int 			index_b;
	// int 			index_f;


	/* copy sensor data to working buffer */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		source[i] =  f[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	ii = 0;
	while (ii < _NUMBER_OF_SENSORS){

		/* find low value */
		for(i=0; i<_NUMBER_OF_SENSORS-1;i++){
			if (source_deleted[i] == _TRUE)
				continue;
			if(source[i].sensor_id <= source[i+1].sensor_id)
				index_b = source[i].sensor_id;
			else 
				index_b = source[i+1].sensor_id;
		}

		destination[ii++] = source[index_b];
		source_deleted[index_b] = _TRUE;
	}

	/* copy sorted sensor data to shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		f[i] = destination[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	return 0;
}

int s_save(_CMD_FSM_CB *cb){ 

	_SEN_DAT 			buf[_NUMBER_OF_SENSORS];
	int 				i;

	/* copy sensor data to working buffer */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		buf[i] = cb->ipc_ptr->s_dat[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* check for empty space */
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		if(buf[i].active == _FALSE){
			buf[i].active = _TRUE;
			buf[i].sensor_id = cb->w_sen_dat.sensor_id;
			strcpy(buf[i].name, cb->w_sen_dat.name);
			strcpy(buf[i].description, cb->w_sen_dat.description);
		}
		else{
			return 1;
		}

	s_sort(buf);

	/* copy sensor data to shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		cb->ipc_ptr->s_dat[i] = buf[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	return 0;
}

int s_delete(){

	return 0;
}