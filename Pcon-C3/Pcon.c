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
 
#include "Pcon.h"

#include "test.h"


#define RX                      1
#define TX                      0
#define MODE                    0
#define BAUD                 9600
#define SERIAL_BUFFER_SIZE    128
#define START_COMMAND         27

// /*************************** drivers  ********************************/
// extern _Driver _FullDuplexSerialDriver;
// extern _Driver _FileDriver;
// _Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL}; 

/***************************** externals ******************************/

int main(int argc, char *argv[]){
    // FILE        *C3in;
    // FILE        *C3out;

    int32_t   Val;

    sleep(1);
    printf("\033\143"); //clear the terminal screen, preserve the scroll back
    disp_sys();


    waitcnt(CLKFREQ+CNT);
    printf("start\n");
    fdsSpin_Start(31, 30, 0, 115200);
    fdsSpin_Str("Hello World\r");
    while (1) {
        Val = fdsSpin_Rx();
        fdsSpin_Tx(Val);
    }

    // C3in  = fopen("C3:9600, 1, 0", "rb");
    // C3out = fopen("C3:9600, 1, 0", "wb"); 

    // printf("C3in = %u, C3out = %u\n",C3in, C3out);

    // printf("Opened Serial Port 1/0 setting raw IO\n");     
    // setvbuf(C3in, NULL, _IONBF, 0);
    // setvbuf(C3out, NULL, _IONBF, 0);
    // C3in->_flag &= ~_IOCOOKED;

    printf("\nnormal termination\n");
    return 0;

	// int         in_byte;
 //    int         out_byte;

    // FdSerial_t      *serial_port;




    // _rxpin = RX;
    // _txpin = TX;
    // _baud = BAUD;

    // serial_port = fopen("serial.port","w");
    // printf(" fopen returned <%u>\n",(uint32_t)serial_port);

    return 0;

    // serial_port = s_open();
    while(1){
        // in_byte = s_rbyte(serial_port);

    }   
    // term(serial_port);
    printf("\nnormal termination\n");
	return 0;
}

void disp_sys(void) {
	printf("\n*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

// void term(FdSerial_t *fd){
//     s_close(fd);
//     printf("serial port closed \n");
//     return;
// }


