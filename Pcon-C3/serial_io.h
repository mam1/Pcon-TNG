/*
 * serial_io.h
 *
 *  Created on: Nov 23, 2014
 *      Author: mam1
 */

#ifndef SERIAL_IO_H_
#define SERIAL_IO_H_


#define BAUD 			9600   			//Change as needed
// #define MODEMDEVICE "/dev/ttyO1"	//Beaglebone Black serial port
// #define _POSIX_SOURCE 1 			//POSIX compliant source */
// #define FALSE 0
// #define TRUE 1
#define RX				1
#define TX				0
#define MODE 			0
#define READ_TRYS       100
#define SREAD			1
#define SWRITE			0

FdSerial_t 	*s_open(void);
void    	s_close(FdSerial_t *);
uint8_t     s_rbyte(FdSerial_t *);
void    	s_wbyte(FdSerial_t *, uint8_t *);
void    	s_error(FdSerial_t *);

#endif /* SERIAL_IO_H_ */
