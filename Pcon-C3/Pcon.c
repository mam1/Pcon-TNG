/*
 * Pcon.c
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */


#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include "propeller.h"
#include "full_duplex_serial_ht.h"
// #include <simpletools.h>
// #include <fdserial.h>
#include "Pcon.h"

#define RX                      1
#define TX                      0
#define MODE                    0
#define BAUD                 9600
#define SERIAL_BUFFER_SIZE    128
#define START_COMMAND         27

/*************************** drivers  ********************************/
// extern _Driver _FullDuplexSerialDriver;
// extern _Driver _FileDriver;
// _Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL};
/***************************** external ******************************/

int main(int argc, char *argv[]){
	char		temp;
	int			i;

	char		*jackSays = "one more time";

    fdx_start(30, 31, 115200);
    fdx_puts("one mre time"); 


	sleep(1);
	disp_sys();

/* setup full duplex serial */
    _DIRA = 0;	// Give up the pin for the fds driver
    _OUTA = 0;	// Give up the pin for the fds driver
    // fdx_start(1, 0, 115200);

    fdx_start(30, 31, 115200);

    fdx_puts(jackSays); 

/* echo */
    temp = fdx_rx();
        for (i = 0; i < 8; i++)
        {
            if (temp & 0x80)
            {
                fdx_tx('1'); 
            }
            else
            {
                fdx_tx('0');
            }
            temp <<= 1;
        }
        fdx_tx('\n');

	return 0;
}

void disp_sys(void) {
	printf("\033\143"); 					//clear the terminal screen, preserve the scroll back
	printf("\n*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

