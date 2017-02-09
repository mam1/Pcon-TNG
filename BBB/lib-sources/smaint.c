/*****************************************************************/
/*  smaint.c -                                                   */
/*  routines to maintain the sensor list                         */
/*                                                               */
/*                                                               */
/*****************************************************************/

int s_search(int id, _SEN_DAT *s)
{
	int 		i;

	for ( i = 0; i < _NUMBER_OF_SENSORS; i++)
		if (cb->ipc_ptr->s_dat[i].sensor_id == id)
		{
			cb->w_sen_dat.id = id;
			cb->w_sen_dat.name = cb->ipc_ptr->s_dat[i].name;
			cb->w_sen_dat.description = cb->ipc_ptr->s_dat[i].description;
			return 0;
		}
	
	cb->w_sen_dat.id = id;
	cb->w_sen_dat.name = cb->ipc_ptr->s_dat[i].name;
	cb->w_sen_dat.description = cb->ipc_ptr->s_dat[i].description;

	return 0;
}
_
int s_sort(_SEN_DAT *f)
{
	_SEN_DAT 		source[_NUMBER_OF_SENSORS];
	_SEN_DAT 		destination[_NUMBER_OF_SENSORS];
	int 			source_deleted[_NUMBER_OF_SENSORS];
	int 			i,ii;
	int 			index_b, index_f;

	/* copy sensor data to working buffer */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		source[i] = f->sen_dat[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	ii = 0;
	while (ii < _NUMBER_OF_SENSORS){

		/* find low value */
		for(i=0; i<_NUMBER_OF_SENSORS-1;i++){
			if (source_deleted[i] == TRUE)
				continue;
			if(source[i].sensor_id <= source[i+1].sensor_id)
				index_b = source[i].sensor_id;
			else 
				index_b = source[i+1].sensor_id;
		}

		destination[ii++] = source[index_b];
		source_deleted[index_b] = TRUE;
	}

	/* copy sorted sensor data to shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		*f[i] = destination[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	return 0;
}

int s_add(_SEN_DAT *f[_NUMBER_OF_SENSORS], _CMD_FSM_CB *cb){ 

	_SEN_DAT 			buf[_NUMBER_OF_SENSORS];

	/* copy sensor data to working buffer */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		buf[i] = f->sen_dat[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* check for empty space */
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		if(buf[i].active == FALSE){
			buf[i].active = TRUE
			buf[i].sensor_id = cb->w_sen_dat.sensor_d;
			buf[i].name = cb->w_sen_dat.name;
			buf[i].description =  cb->w_sen_dat.description;
			return 0;
	}
	return 1;
}


	s_sort(buf);

	/* copy sensor data to shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	for(i=0; i<_NUMBER_OF_SENSORS;i++)
		*f[i] = buf[i];
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	return 0;
}

int s_delete()