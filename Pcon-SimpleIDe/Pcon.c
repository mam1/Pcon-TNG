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
#define PING         27
#define ACK           6

int main()                                    // main function
{
  char c;
  int i;
  struct{
    int       size;
    uint8_t   action;     // 0=read, 1=write
    uint8_t   buf[SERIAL_BUFFER_SIZE];
  } iob;    

  fdserial    *serial_connection;
  int         cmd_code;
  uint8_t     *bptr, cc;
  
  cmd_code = '7';
  cc = (uint8_t)6;
  print("Serial test is running\n");
  serial_connection = fdserial_open(RX, TX, MODE, BAUD);
  print("fdserial_open retuned<%d>\n",serial_connection);
  print("waiting for serial port\n");
  while(1){
    c = readChar(serial_connection);
    print("recieved <%u>\n",c);
    switch(c){
      case PING:
        writeChar(serial_connection, cc);  
        printf("sent <%u>\n\n",cc);
        iob.action = readChar(serial_connection);
        printf("recieved <%u>\n",iob.action);
        if(c){
          bptr = (uint8_t *)&iob.size;
          for(i=0;i<4;i++)*bptr++ = readChar(serial_connection);   
          printf("size = %i\n",iob.size);
          bptr = iob.buf;
          for(i=0;i<iob.size;i++)*bptr++ = readChar(serial_connection);\
          printf("mesage <%s>\n",iob.buf);
        }                          
        break;
      default:
        printf("unrecognized command\n");
      }        
    } 
  return 0;       
}
