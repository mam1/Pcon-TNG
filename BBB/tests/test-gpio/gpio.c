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
/* files within sysfs */

#define BONE_SLOTS   "/sys/devices/platform/bone_capemgr/slots"
#define GPIO_VALUE   "/sys/class/gpio/gpio%d/value"
#define ANALOG_VALUE "/sys/devices/platform/ocp/44e0d000.tscadc/TI-am335x-adc/iio:device0/in_voltage%d_raw"

/* gpio memory layout in three sections */

#define GPIO0_START_ADDR    0x44e07000
#define GPIO0_END_ADDR      0x44e08000
#define GPIO0_SIZE          (GPIO0_END_ADDR - GPIO0_START_ADDR)

#define GPIO1_START_ADDR    0x4804C000
#define GPIO1_END_ADDR      0x4804DFFF
#define GPIO1_SIZE          (GPIO1_END_ADDR - GPIO1_START_ADDR)

#define GPIO2_START_ADDR    0x41A4C000
#define GPIO2_END_ADDR      0x41A4D000
#define GPIO2_SIZE          (GPIO2_END_ADDR - GPIO2_START_ADDR)

#define GPIO3_START_ADDR    0x41A4E000
#define GPIO3_END_ADDR      0x41A4F000
#define GPIO3_SIZE          (GPIO3_END_ADDR - GPIO3_START_ADDR)

// #define GPIO_OE             0x134
// #define GPIO_SETDATAOUT     0x194
// #define GPIO_CLEARDATAOUT   0x190

#define GPIO_OE             0x14d
#define GPIO_SETDATAOUT     0x14e
#define GPIO_CLEARDATAOUT   0x14f


#define REG                 volatile unsigned int


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

/* GPIO 1 */

#define USR0_LED            (1<<21)
#define USR1_LED            (1<<22)
#define USR2_LED            (1<<23)
#define USR3_LED            (1<<24)

#define P8_3      6
#define P8_4      7
#define P8_5      2
#define P8_6      3
#define P8_11     13
#define P8_12     12
#define P8_15     15
#define P8_16     14
#define P8_20     31
#define P8_21     30
#define P8_22     5
#define P8_23     4
#define P8_24     1
#define P8_25     0
#define P8_26     29
#define P9_12     28
#define P9_15     16
#define P9_23     17
#define GPIO1_38  P8_3
#define GPIO1_39  P8_4
#define GPIO1_34  P8_5
#define GPIO1_35  P8_6
#define GPIO1_45  P8_11
#define GPIO1_44  P8_12
#define GPIO1_47  P8_15
#define GPIO1_46  P8_16
#define GPIO1_63  P8_20
#define GPIO1_62  P8_21
#define GPIO1_37  P8_22
#define GPIO1_36  P8_23
#define GPIO1_33  P8_24
#define GPIO1_32  P8_25
#define GPIO1_61  P8_26
#define GPIO1_60  P9_12
#define GPIO1_48  P9_15
#define GPIO1_49  P9_23

/* GPIO 2 */

#define P8_18     1
#define P8_27     22
#define P8_28     24
#define P8_29     23
#define P8_30     25
#define P8_39     12
#define P8_40     13
#define P8_41     10
#define P8_42     11
#define P8_43     8
#define P8_44     9
#define P8_45     6
#define P8_46     7
#define GPIO2_65  P8_18
#define GPIO2_86  P8_27
#define GPIO2_88  P8_28
#define GPIO2_87  P8_29
#define GPIO2_89  P8_30
#define GPIO2_76  P8_39
#define GPIO2_77  P8_40
#define GPIO2_74  P8_41
#define GPIO2_75  P8_42
#define GPIO2_72  P8_43
#define GPIO2_73  P8_44
#define GPIO2_70  P8_45
#define GPIO2_71  P8_46

/* GPIO 3 */

#define P9_25     21
#define P9_27     19
#define GPIO3_117 P9_25
#define GPIO3_115 P9_27

#define P8 0
#define P9 1


/**
 * @brief Given the header and pin number return the gpio number
 * @param header Either P8 or P9
 * @param pin Pin number on the header (not the gpio number)
 * @return The gpio number
 */
int header_pin(int header, int pin)
{
    int i;
    int p8_gpio1[] = {3, 4, 5, 6, 11,12,15,16,20,21, 22,23,24,25,26,12};
    int p8_gpio1_pin[] = {P8_3, P8_4, P8_5, P8_6, P8_11,P8_12,P8_15,P8_16,P8_20,P8_21, P8_22,P8_23,P8_24,P8_25,P8_26,P8_12};
    int p8_gpio2[] = {18,27,28,29,30,39,40,41,42,43, 44,45,46};
    int p8_gpio2_pin[] = {P8_18,P8_27,P8_28,P8_29,P8_30,P8_39,P8_40,P8_41,P8_42,P8_43, P8_44,P8_45,P8_46};
    int p9_gpio1[] = {12,15,23};
    int p9_gpio1_pin[] = {P9_12,P9_15,P9_23};
    int p9_gpio3[] = {25,27};
    int p9_gpio3_pin[] = {P9_25,P9_27};

    if (header == P8) {
        for (i = 0; i < 16; i++)
            if (pin == p8_gpio1[i]) return p8_gpio1_pin[i];
        for (i = 0; i < 13; i++)
            if (pin == p8_gpio2[i]) return p8_gpio2_pin[i];
    }
    else {
        for (i = 0; i < 3; i++)
            if (pin == p9_gpio1[i]) return p9_gpio1_pin[i];
        for (i = 0; i < 2; i++)
            if (pin == p9_gpio3[i]) return p9_gpio3_pin[i];
    }

    return 0;
}

/* initialise gpio pins for iolib */
int init_gpio(void) {
	// char 				command[120];
	// int 				i;
	// int 				gpios[_NUMBER_OF_GPIOS] = {_GPIOS};

	// for (i = 0; i < _NUMBER_OF_GPIOS; i++) {
	// 	sprintf(command, "if [ ! -d /sys/class/gpio/gpio%i ]; then echo %i > /sys/class/gpio/export; fi", gpios[i], gpios[i] );
	// 	// logit(NULL, command);
	// 	printf("system command %s returned %i\n", command, system(command));
	// }

	return 0;
}


int dout(){

	return 0;
}

int main(void){

	int 		rnum,state;
	char 		header[10];
	char 		pin[10];
	char 		cmd[30];

	int 			i;


	typedef struct {
		int         header;
		int         pin;
	} _GPIO;

	_GPIO 			chan[16] = {_CHAN0,_CHAN1,_CHAN2,_CHAN3,_CHAN4,_CHAN5,_CHAN6,_CHAN7,_CHAN8,_CHAN9,_CHAN10,_CHAN11,_CHAN12,_CHAN13,_CHAN14,_CHAN15};
	_GPIO 			heart[2] = {_HB0, _HB1};

	for(i=0; i<16; i++){
		printf("  P%i.%i .... %i\n", chan[i].header, chan[i].pin, header_pin(chan[i].header, chan[i].pin));
	}

	for(i=0; i<2; i++){
		printf("  P%i.%i .... %i\n", heart[i].header, heart[i].pin, header_pin(heart[i].header, heart[i].pin));
	}


	// show_gpio();
									
	// printf("enter number > 17 to exit\n");
	// for(;;){
	// 	printf("enter relay number 0 - 17 > ");
	// 	scanf ("%d",&rnum);
	// 	if ((rnum < 0) || (rnum > 17)){
	// 		printf("\nnormal exit\n");
	// 		return 0;
	// 	}
	// 	sprintf( header, "%d", gpio[rnum].header );
	// 	sprintf( pin, "%d", gpio[rnum].pin );


	// 	state = -1;
	// 	while((state != 0) && (state != 1)){
	// 		printf("enter 0 for off or 1 for on > ");
	// 		scanf ("%d",&state);
	// 		if ((state == 0) || (state == 1)){
	// 			printf("led %i on header %i, pin %i set to %i\n", rnum, gpio[rnum].header, gpio[rnum].pin, state );


	// 			char 	highlow[5];
	// 			char 	*prefix="config-pin P";

	// 			switch(state){
	// 				case 0:
	// 					strcpy(highlow," low");
	// 					break;
	// 				case 1:
	// 					strcpy(highlow," hi");
	// 					break;
	// 				default:
	// 					break;
	// 			}

	// 			strcpy(cmd,prefix);
	// 			strcat(cmd,header);
	// 			strcat(cmd,".");
	// 			strcat(cmd,pin);
	// 			strcat(cmd,highlow);
	// 			printf("\n\n%s\n\n",cmd);
	// 			// digital_output(gpio[rnum].header, gpio[rnum].pin, state);
	// 			system(cmd);
	// 			show_gpio();
	// 		}









	// 	}
	// }

	return 0;
}
