/*
 * Pcon.c
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */


#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
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

	// fdserial			*bbb;
//	bbb =  fdserial_open(RX, TX, MODE, BAUD);
	sleep(1);
	disp_sys();

	return 0;
}

void disp_sys(void) {
	printf("\033\143"); 					//clear the terminal screen, preserve the scroll back
	printf("\n*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

