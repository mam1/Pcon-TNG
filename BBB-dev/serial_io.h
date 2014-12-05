/*
 * serial_io.h
 *
 *  Created on: Nov 23, 2014
 *      Author: mam1
 */

#ifndef SERIAL_IO_H_
#define SERIAL_IO_H_


#define BAUDRATE B9600   			//Change as needed, keep B
#define MODEMDEVICE "/dev/ttyO1" 	//Beaglebone Black serial port
#define _POSIX_SOURCE 1 			//POSIX compliant source */
#define FALSE 0
#define TRUE 1

int s_open(void);
void s_close(int);



#endif /* SERIAL_IO_H_ */
