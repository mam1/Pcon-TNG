/*

	see how many gpio pins I can use

*/

// #include <stdio.h>
// #include <stdint.h>		//uint_8, uint_16, uint_32, etc.
// #include <sys/types.h>
// #include <sys/stat.h> 

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

// #include "iolib.h"
#include "bbb.h"

/* gpio enable list */
#define _NUMBER_OF_GPIOS	17
#define _GPIOS              38,39,34,35,66,67,69,68,45,44,23,26,47,46,27,65,22	
#define _HEADERS			 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
#define _PINS  				 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19 		

/* map P8 header pins */
#define _LED_1			7	
#define _LED_2			9		
#define _LED_3			8			
#define _LED_4			10
#define _DIOB_DIN 		11
#define _DIOB_DATA_RLY	11
#define _DIOB_SCLK_IN	12
#define	_DIOB_SCLK_RLY	12
#define	_DIOB_LAT_RLY	14
#define	_DIOB_LOAD_IN	27
#define _R1_CAPE		16
#define _R2_CAPE		17
#define _R3_CAPE		18
#define _R4_CAPE		26
 
/* map P9 header pins  */
#define _R5_CAPE		12
#define _R6_CAPE		15
#define _R7_CAPE		23
#define _R8_CAPE		41

/* initialise gpio pins */
int init_gpio(void) {
	char 				command[120];
	int 				i;
	int 				gpios[_NUMBER_OF_GPIOS] = {_GPIOS};

	for (i = 0; i < _NUMBER_OF_GPIOS; i++) {
		sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpios[i], gpios[i] );
		// logit(command);
		printf("system command %s returned %i\n", command, system(command));
	}

	return 0;
}

/* get a line of input from the console */
// char *getline(void) {
//     char 		*line = malloc(100), *linep = line;
//     size_t 		lenmax = 100, len = lenmax;
//     int 		c;

//     if(line == NULL)
//         return NULL;

//     for(;;) {
//         c = fgetc(stdin);
//         if(c == EOF)
//             break;

//         if(--len == 0) {
//             len = lenmax;
//             char * linen = realloc(linep, lenmax *= 2);

//             if(linen == NULL) {
//                 free(linep);
//                 return NULL;
//             }
//             line = linen + (line - linep);
//             linep = linen;
//         }

//         if((*line++ = c) == '\n')
//             break;
//     }
//     *line = '\0';
//     return linep;
// }

int main(void){

	int 		rnum,state;


	// switch (*getline()){
	// 	case



	// }

	/* initialise gpio access */
	// iolib_init();



	// init_gpio();
	// iolib_init();



	// /* setup gpio access to LEDs on the WaveShare Cape*/
	// logit("iolib initialized");
	// logit("mapping WaveShare Misc Cape leds to gpio pins");
	// iolib_setdir(8, _LED_1, BBBIO_DIR_OUT);
	// iolib_setdir(8, _LED_2, BBBIO_DIR_OUT);
	// iolib_setdir(8, _LED_3, BBBIO_DIR_OUT);
	// iolib_setdir(8, _LED_4, BBBIO_DIR_OUT);

	// /* turn off the LEDs */
	// pin_low(8, _LED_1);
	// pin_low(8, _LED_2);
	// pin_low(8, _LED_3);
	// pin_low(8, _LED_4);

	// /* setup gpio access to PhotoMos relays */
	// logit("mapping PhotoMos relays to gpio pins");
	// iolib_setdir(8, _R1_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(8, _R2_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(8, _R3_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(8, _R4_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R5_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R6_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R7_CAPE, BBBIO_DIR_OUT);
	// iolib_setdir(9, _R8_CAPE, BBBIO_DIR_OUT);

	// /* turn all relays off */
	// pin_low(8, _R1_CAPE);
	// pin_low(8, _R2_CAPE);
	// pin_low(8, _R3_CAPE);
	// pin_low(8, _R4_CAPE);
	// pin_low(9, _R5_CAPE);
	// pin_low(9, _R6_CAPE);
	// pin_low(9, _R7_CAPE);
	// pin_low(9, _R8_CAPE);

	// /* setup gpio access to serial header on the DIOB */
	// logit("mapping DIOB serial header to gpio pins");
	// iolib_setdir(8, _DIOB_DIN, BBBIO_DIR_OUT);
	// iolib_setdir(8, _DIOB_SCLK_IN, BBBIO_DIR_OUT);
	// iolib_setdir(8, _DIOB_LAT_RLY, BBBIO_DIR_OUT);
	// pin_low(8, _DIOB_SCLK_IN);
	// pin_low(8, _DIOB_DIN);

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
			if ((state == 0) || (state == 1))
				printf("rnum %i set to %i\n", rnum, state);
				digital_output(P9, 12, 1);
			    sleep(1);
			    digital_output(P9, 12, 0);
		}

	}

	return 0;
}
