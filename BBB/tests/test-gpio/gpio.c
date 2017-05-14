/*

	see how many gpio pins I can use

*/

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "bbb.h"


/*	gpio pin assignment {header,pin}  	*/
#define _RLY1			{9,12}
#define _RLY2			{9,15}
#define _RLY3			{9,23}
#define _RLY4			{8,27}



int main(void){

	int 		rnum,state;

	typedef struct {
		int         header;
		int         pin;
	} _GPIO;

	_GPIO 			gpio[19] = {_RLY1,_RLY2,_RLY3,_RLY4};


	show_gpio();
									
	printf("enter number > 17 to exit\n");
	for(;;){
		printf("enter relay number 0 - 17 > ");
		scanf ("%d",&rnum);
		if ((rnum < 0) || (rnum > 17)){
			printf("\nnormal exit\n");
			return 0;
		}
		state = -1;
		while((state != 0) && (state != 1)){
			printf("enter 0 for off or 1 for on > ");
			scanf ("%d",&state);
			if ((state == 0) || (state == 1)){
				printf("led %i on header %i, pin %i set to %i\n", rnum, gpio[rnum].header, gpio[rnum].pin, state );
				digital_output(gpio[rnum].header, gpio[rnum].pin, state);
			}

		}
	}

	return 0;
}
