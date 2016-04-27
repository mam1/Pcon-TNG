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

int tpid(int t, int c){
	
		if(t < c)
			return 0;
		if(t > c )
			return 1;

	return -1;
}


int main(void){

	int  		target, current;

	printf("\n *** test pid code ***\n\n");
	printf("enter target -1 to exit\n");

	while(1){
		printf(" enter tagret temp > ");
		scanf ("%d",&target);
		if(target < 0)
			return 0;
		printf(" enter current temp > ");
		scanf ("%d",&current);

		printf(" tpid given target %i and current %i returned %i\n\n",target,current, tpid(target, current));

	}
}