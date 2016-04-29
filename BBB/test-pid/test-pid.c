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
#include "slab_control.h"

// int 	hold_temp;

// int direction(int n){

// 	static int 		hold;

// 	if(hold == n)
// 		return 0;

// 	if(hold > n){
// 		hold = n;
// 		return -1;
// 	}
// 	hold = n;
// 	return 1;
// }

// int tpid(int target, int actual){
// 	if(direction(actual) == -1){ // actual falling
// 		if(target < actual)
// 			return 0;
// 		if(target >= actual)
// 			return 1;
// 		else
// 			return 0;
// 	}
// 	else{						// actual rising
// 		if(target < actual + 10)
// 			return 0;
// 		if(target > actual +10)
// 			return 1;
// 		else
// 			return 0;
// 	}
// 	printf(" ************* error \n");
// }


int main(void){

	int  		target, current, i;

	printf("\n ******************* test pid code ******************\n");

	target = 100;
	printf("\n target  current  returned \n");
	for(current=88;current<125;current++)
		printf("%6i%8i%9i\n",target,current, slabcon(target, current));

	printf("\n target  current returned \n");
	for(current=125;current>88;current--)
		printf("%6i%8i%9i\n",target,current, slabcon(target, current));

	printf("\n target  current returned \n");
	for(current=88;current<125;current++)
		printf("%6i%8i%9i\n",target,current, slabcon(target, current));

	printf("\n target  current returned \n");
	for(current=125;current>88;current--)
		printf("%6i%8i%9i\n",target,current, slabcon(target, current));


	// printf("enter target -1 to exit\n");

	// while(1){
	// 	printf(" enter tagret temp  > ");
	// 	scanf ("%d",&target);
	// 	if(target < 0)
	// 		return 0;
	// 	printf(" enter current temp > ");
	// 	scanf ("%d",&current);

	// 	printf(" tpid given target %i and current %i returned %i\n\n",target,current, tpid(target, current));

	// }
	return 0;
}