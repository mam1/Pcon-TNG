/*****************************************************************/
/*                                                               */
/*                                                               */
/*                                                               */
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
	}
	
	cb->w_sen_dat.id = id;
	cb->w_sen_dat.name = cb->ipc_ptr->s_dat[i].name;
	cb->w_sen_dat.description = cb->ipc_ptr->s_dat[i].description;

	return 0;
}