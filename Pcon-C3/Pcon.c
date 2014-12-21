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

	sleep(1);
    printf("\033\143"); //clear the terminal screen, preserve the scroll back
	disp_sys();

/* setup full duplex serial */
    _DIRA = 0;			//set pins to input (0) or output (1)
    _OUTA = 0;			//set output pin states when corresponding DIRA bits are 1
    fdx_start(RX, TX, BAUD);

/* echo */
    printf("start echo\n");
    while((temp = fdx_rx()) == '\0');
    printf("recieved %u\n",temp);
    while(temp != 'q'){
        // fdx_tx('\n');
        // fdx_tx('\r');
        // fdx_tx('<'); 
        fdx_tx(temp);
        printf("recieved %u\n",temp);
        // fdx_tx('>');
        // fdx_tx('\n');
        // fdx_tx('\r'); 
        temp = fdx_rx(); 
    }   
        

    printf("\nnormal termination\n");
	return 0;
}

void disp_sys(void) {
	printf("\n*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

