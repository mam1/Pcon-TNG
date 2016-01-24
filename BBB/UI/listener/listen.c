#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "typedefs.h"
#include "ipc.h"

IPC_DAT 		ipc_dat; 							//ipc data
void			*data;								//pointer to ipc data
int 			fd;					//file descriptor for ipc data file

int main(void){

	printf("\n*** listen ***\n\n");

    /* set up file mapped shared memory for inter process communication */
    fd = ipc_open(_IPC_FILE);					// create/open ipc file
	data = ipc_map(fd,ipc_size());				// map file to memory
	memcpy(&ipc_dat,data,sizeof(ipc_dat));		// move shared memory data to local structure
	printf(" %i bytes of data moved\n",sizeof(ipc_dat));
	printf("starting loop with force_update - %i\n",ipc_dat.force_update);
	while(1){
		if(ipc_dat.force_update){
			ipc_dat.force_update = 0;
			memcpy(data,&ipc_dat,sizeof(ipc_dat));  // move local data into shared memory
			printf(" %i bytes of data moved\n",sizeof(ipc_dat));
			printf("update forced, flag cleared\n");
		}
		memcpy(&ipc_dat,data,sizeof(ipc_dat));		// move shared memory data to local structure
	}

	return 0;
}