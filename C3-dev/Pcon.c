/*
  readUART.c

*/
#include "propeller.h"
#include "stdint.h"		//uint_8, uint_16, uint_32, etc.
#include "simpletools.h"                      // Include simpletools
#include "fdserial.h"

//#define IN                      0
//#define OUT                     1
#define RX                      1
#define TX                      0
#define MODE                    0
#define BAUD                 9600
#define SERIAL_BUFFER_SIZE    128
#define START_COMMAND         27


int get_serial(fdserial *port,int bcnt,uint8_t buf[SERIAL_BUFFER_SIZE]){
  return 0;
}
  
int put_serial(fdserial *port,int bcnt,uint8_t buf[SERIAL_BUFFER_SIZE]){
  return 0;
}
int main()                                    // main function
{
  
  
  int c,i; 
  fdserial    *serial_connection;
  uint8_t      serial_buffer[SERIAL_BUFFER_SIZE],*buf_ptr;
  int         cmd_code;
  buf_ptr = serial_buffer;
  print("Serial test is running\n");
  serial_connection = fdserial_open(RX, TX, MODE, BAUD);
  print("fdserial_open retuned<%d>\n",serial_connection);
  writeChar(serial_connection, CLS);
  print("CLS written to serial connection\n");
  *buf_ptr = fdserial_rxChar(serial_connection);
  print("got a character from serial port <%c>\n",*buf_ptr);
  while(*buf_ptr != 'q'){
    buf_ptr = serial_buffer;
          print("1\n");
    while(*buf_ptr != START_COMMAND){
      *buf_ptr = fdserial_rxChar(serial_connection);
          print("2\n");
          print("got a character from serial port <%c>\n",*buf_ptr);
      }          
      if(*buf_ptr == START_COMMAND){
        print("exit detected\n program terminated\n");
         exit(1);
    }
    print("serial buffer (%s)\n",serial_buffer);
    for(i=0;i<255;i++) serial_buffer[i] = '\0';
  }          
    
    
  print("waiting for serial port\n");
  for(i=0;i<10;i++){
    *buf_ptr = fdserial_rxChar(serial_connection);
    print("%c",*buf_ptr++);
  }
  *buf_ptr = '\0';
  print("\n");
  print("serial buffer <%s>",serial_buffer); 
     
}
