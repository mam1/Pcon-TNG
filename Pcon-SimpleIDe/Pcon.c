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
  int         cmd_code;
  print("Serial test is running\n");
  serial_connection = fdserial_open(RX, TX, MODE, BAUD);
  print("fdserial_open retuned<%d>\n",serial_connection);
  writeChar(serial_connection, CLS);
  print("CLS written to serial connection\n");
  print("waiting for serial port\n");
while(1){
    c = fdserial_rxChar(serial_connection);
    print("recieved %c\n",c);
//    fdserial_txChar(serial_connection,c);
    writeChar(serial_connection, c);  
    printf("sent %c\n\n",c);
  }
  print("\n");

     
}
