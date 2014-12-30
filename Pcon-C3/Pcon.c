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
#include "FdSerial.h"
#include "serial_io.h"
#include "Pcon.h"

#define RX                      1
#define TX                      0
#define MODE                    0
#define BAUD                 9600
#define SERIAL_BUFFER_SIZE    128
#define START_COMMAND         27

/*************************** drivers  ********************************/
extern _Driver _FullDuplexSerialDriver;
extern _Driver _FileDriver;
_Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL}; 

/***************************** externals ******************************/
extern unsigned int _rxpin;
extern unsigned int _txpin;
extern unsigned int _baud;

int main(int argc, char *argv[]){
	// int         in_byte;
 //    int         out_byte;
    // FdSerial_t      *serial_port;

    FILE                *serial_port;


	sleep(1);
    printf("\033\143"); //clear the terminal screen, preserve the scroll back
	disp_sys();

    _rxpin = RX;
    _txpin = TX;
    _baud = BAUD;

    serial_port = fopen("serial.port","w");
    printf(" fopen returned <%u>\n",(uint32_t)serial_port);


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


