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

// #include "bbb.h"


/*	gpio pin assignment {header,pin}  	*/
#define _CHAN0			{8,3}
#define _CHAN1			{8,4}
#define _CHAN2			{8,5}
#define _CHAN3			{8,6}
#define _CHAN4			{8,11}
#define _CHAN5			{8,12}
#define _CHAN6			{8,15}
#define _CHAN7			{8,16}
#define _CHAN8			{8,18}
#define _CHAN9			{8,20}
#define _CHAN10			{8,21}
#define _CHAN11			{8,22}
#define _CHAN12			{8,23}
#define _CHAN13			{8,24}
#define _CHAN14			{8,25}
#define _CHAN15			{8,26}

#define _HB0 			{8,27}
#define _HB1 			{8,28}

init_gpio

int dout(){

	return 0;
}

int main(void){

	int 		rnum,state;
	char 		header[10];
	char 		pin[10];
	char 		cmd[30];

	typedef struct {
		int         header;
		int         pin;
	} _GPIO;

	_GPIO 			chan[16] = {_CHAN0,_CHAN1,_CHAN2,_CHAN3,_CHAN4,_CHAN5,_CHAN6,_CHAN7,_CHAN8,_CHAN9,_CHAN10,_CHAN11,_CHAN12,_CHAN13,_CHAN14,_CHAN15,_HB};
	_GPIO 			heart[2] = {_HB0, _HB1};



	show_gpio();
									
	printf("enter number > 17 to exit\n");
	for(;;){
		printf("enter relay number 0 - 17 > ");
		scanf ("%d",&rnum);
		if ((rnum < 0) || (rnum > 17)){
			printf("\nnormal exit\n");
			return 0;
		}
		sprintf( header, "%d", gpio[rnum].header );
		sprintf( pin, "%d", gpio[rnum].pin );


		state = -1;
		while((state != 0) && (state != 1)){
			printf("enter 0 for off or 1 for on > ");
			scanf ("%d",&state);
			if ((state == 0) || (state == 1)){
				printf("led %i on header %i, pin %i set to %i\n", rnum, gpio[rnum].header, gpio[rnum].pin, state );


				char 	highlow[5];
				char 	*prefix="config-pin P";

				switch(state){
					case 0:
						strcpy(highlow," low");
						break;
					case 1:
						strcpy(highlow," hi");
						break;
					default:
						break;
				}

				strcpy(cmd,prefix);
				strcat(cmd,header);
				strcat(cmd,".");
				strcat(cmd,pin);
				strcat(cmd,highlow);
				printf("\n\n%s\n\n",cmd);
				// digital_output(gpio[rnum].header, gpio[rnum].pin, state);
				system(cmd);
				show_gpio();
			}









		}
	}

	return 0;
}
