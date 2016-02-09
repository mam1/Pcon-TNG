/**
 * This is the main prop-echo program file.
 */

#include <stdio.h>
#include <simpletools.h>                    // Library include
#include <fdserial.h>

#define _RX 1
#define _TX 0
#define _MODE 0
#define _BAUD 9600

fdserial 			*Serial;

int main(void)
{
  int             c; 
  Serial = fdserial_open(_RX,_TX,_MODE,_BAUD);
  printi("Serial = <%d>\n",Serial); 		
  printi("\nstarting prop echo\n");
  for(;;){
    c = fdserial_rxChar(Serial);
    printi("received <%02x>\n", c); 	
  }    
  return 0;
}

