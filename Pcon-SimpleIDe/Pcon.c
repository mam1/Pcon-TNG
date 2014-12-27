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

int main()                                    // main function
{
  char c;
  int i; 
  fdserial    *serial_connection;
  int         cmd_code;
  
  cmd_code = '7';
  print("Serial test is running\n");
  serial_connection = fdserial_open(RX, TX, MODE, BAUD);
  print("fdserial_open retuned<%d>\n",serial_connection);
  print("waiting for serial port\n");
  while(1){
    c = readChar(serial_connection);
    print("recieved <%u>\n",c);
    writeChar(serial_connection, cmd_code);  
    printf("sent <%u>\n\n",cmd_code++);     
  }
  print("\n");

     
}
