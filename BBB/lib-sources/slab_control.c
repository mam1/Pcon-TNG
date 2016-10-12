
/* slab_control version info */
#define _MAJOR_VERSION    10
#define _MINOR_VERSION    4
#define _MINOR_REVISION   0

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define _H_SETBACK		5
#define _L_SETBACK		5

/* compare values for the last two calls */
int direction(int n){
	static int 		hold;

	if(hold == n)		
		return 0;		// no change

	if(hold > n){		
		hold = n;
		return -1;		// moving down
	}
	hold = n;
	return 1;			// moving up
}

/* slab controller */
int slabcon(int target, int actual){
	int 			dir;
	dir = direction(actual);
	if(dir == -1){ 							// actual falling
		if(target <= actual)
			return 0;
		if(target > actual)
			return 1;
		else
			return 0;
	}
	else if(dir == 1){						// actual rising
		if(target < actual + _H_SETBACK)
			return 0;
		if(target >= actual - _L_SETBACK)
			return 1;
		else
			return 0;
	}
	else if(dir == 0){						// actual stable
		if(target < actual + _H_SETBACK)
			return 0;
		if(target >= actual - _L_SETBACK)
			return 1;
		else
			return 0;
	}
	printf(" ************* slab controller error \n");
	return 2000;
}