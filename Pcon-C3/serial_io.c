/*
 * serial_io.c
 *
 *  Created on: Nov 12, 2014
 *      Author: mam1
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>    //uint_8, uint_16, uint_32, etc.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Pcon.h"
#include "FdSerial.h"
#include "serial_io.h"

static FdSerial_t     *serial_port;

FdSerial_t *s_open(void){
    if((_FdSerial_start(serial_port, RX, TX, MODE, BAUD)) != 0){
      printf("serial port opened \n");
      _FdSerial_rxflush(serial_port);
      return serial_port;
    }
    perror("\n*** serial open error ");
    term(NULL);
    return 0;
}

void s_close(FdSerial_t *fd){
    _FdSerial_stop(fd);
    return;
}

uint8_t s_rbyte(FdSerial_t *fd){
  int       i;
  uint8_t   byte;
  i =0;
  while(1){
    byte = _FdSerial_rx(fd);
    if((byte < 0) && (i++ == READ_TRYS)){
      perror("\n*** serial read error ");
      s_error(fd);
    }
    if(byte > 0)
      return byte;
    usleep(10);
  }
  perror("\n*** serial read error no records read ");
  s_error(fd);
  return '\0';
}

void s_wbyte(FdSerial_t *fd, uint8_t *byte){
  int    ret;
  ret = write(fd,byte,1);
  if(ret < 0){
    perror("\n*** serial write error ");
    s_error(fd);
  }
  return;
}

void s_error(FdSerial_t *fd){
  s_close(fd);
  term(3);
  return;
}

