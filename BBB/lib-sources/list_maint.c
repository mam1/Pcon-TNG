
/* list_maint version info */
#define _MAJOR_VERSION    10
#define _MINOR_VERSION    4
#define _MINOR_REVISION   0


void del_elm(int *array_start, int *index, int value, int array_size){
	int 			i;
	int 			*elm, *array_end;

	i = 0;
	elm = array_start;
	array_end = array_start + *index - 1;
	while (i < *index){
		if(*elm == value){
			while(elm < array_end){
				*elm = *(elm+1);
				elm++;
			}
			*index = *index - 1;
			return;
		}

		elm++;
		i++;
	}

	return;
}

void add_elm(int *array_start, int *index, int value, int array_size){
	int 			i;
	int 			*elm;

	elm = array_start;
	if(*index == 0){					// add first element
		// printf("adding first element\n");
		*elm = value;
		(*index)++;
		return;
	}
	i = 0;
	while (i < *index){	
		if(*elm == value){		// already in the list
			return;
		}
		elm++;
		i++;		
	}

	if((*index + 1) < array_size){ 		// check if there is room to add another element
		*(array_start + *index) = value;
		(*index)++;
		return;
	}

	return;
}