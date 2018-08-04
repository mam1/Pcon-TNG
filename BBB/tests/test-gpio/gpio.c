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

/*	gpio pin assignment {header,pin,gpio}  	*/
// #define _CHAN0			{8,3,38}
// #define _CHAN1			{8,4,39}
// #define _CHAN2			{8,5,34}
// #define _CHAN3			{8,6,35}
// #define _CHAN4			{8,11,45}
// #define _CHAN5			{8,12,44}
// #define _CHAN6			{8,15,47}
// #define _CHAN7			{8,16,46}
// #define _CHAN8			{8,18,65}
// #define _CHAN9			{8,20,63}
// #define _CHAN10			{8,21,62}
// #define _CHAN11			{8,22,37}
// #define _CHAN12			{8,23,36}
// #define _CHAN13			{8,24,33}
// #define _CHAN14			{8,25,32}
// #define _CHAN15			{8,26,61}
// #define _HB0 			{8,27,86}
// #define _HB1 			{8,28,88}

/*	gpio pin assignment {header,pin,gpio}  	*/
#define _CHAN0			{8,27,86}
#define _CHAN1			{8,28,88}
#define _CHAN2			{8,29,87}
#define _CHAN3			{8,30,89}
#define _CHAN4			{8,31,10}
#define _CHAN5			{8,32,11}
#define _CHAN6			{8,33,9}
#define _CHAN7			{8,34,81}
#define _CHAN8			{8,35,8}
#define _CHAN9			{8,36,80}
#define _CHAN10			{8,37,78}
#define _CHAN11			{8,38,79}
#define _CHAN12			{8,39,86}
#define _CHAN13			{8,40,77}
#define _CHAN14			{8,41,74}
#define _CHAN15			{8,42,75}
#define _HB0 			{8,43,72}
#define _HB1 			{8,44,73}
#define _HB2 			{8,45,70}
#define _HB3 			{8,46,71}

/* export gpio pin */
int init_gpio(int gpio)
{
	char 				command[120];

	sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpio, gpio);
	printf("system command %s returned %i\n", command, system(command));
	sprintf(command, "echo out > /sys/class/gpio/gpio%i/direction", gpio);
	printf("system command %s returned %i\n", command, system(command));
	sprintf(command, "echo 0 > /sys/class/gpio/gpio%i/value", gpio);
	printf("system command %s returned %i\n", command, system(command));
	return 0;
}


int main(void)
{

	int 			rnum, state;
	char 			header[10];
	char 			pin[10];
	char 			cmd[30];
	char 			command[120];

	int 			i;

	/* gpio assignments */
	typedef struct {
		int         header;
		int         pin;
		int 		gpio;
	} _GPIO;
	static _GPIO 	chan[16] = {_CHAN0, _CHAN1, _CHAN2, _CHAN3, _CHAN4, _CHAN5, _CHAN6, _CHAN7, _CHAN8, _CHAN9, _CHAN10, _CHAN11, _CHAN12, _CHAN13, _CHAN14, _CHAN15};
	static _GPIO 	heart[4] = {_HB0, _HB1, _HB2, _HB3};

	/* initializations */
	printf"\n *********\n gpio test started\n");
	printf("starting initializations\n");

	/* load cape that disables HDMI and gives me back the gpios */
	sprintf(command, "echo 'cape-universalh' > /sys/devices/platform/bone_capemgr/slots");
	printf("command <%s>\n", command);
	printf("command returned %i", system(command);

	/* intialize gpio pins */
	for (i = 0; i < 16; i++) {
	// printf(" P%i.%i", chan[i].header, chan[i].pin);
	init_gpio(chan[i].gpio);
	}

	for (i = 0; i < 4; i++) {
	// printf(" P%i.%i", heart[i].header, heart[i].pin);
	init_gpio(heart[i].gpio);
	}

	printf("\ndone with initializations\ntest pins\n");

	for (i = 0; i < 16; i++) {
	sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", chan[i].gpio);
		printf("system command %s returned %i\n", command, system(command));
	}

	for (i = 0; i < 4; i++) {
	sprintf(command, "echo 1 > /sys/class/gpio/gpio%i/value", heart[i].gpio);
		printf("system command %s returned %i\n", command, system(command));
	}

	printf("normal termination\n");

	return 0;
}
