/**
 * This is the main Tprop program file.
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
  int             rxpin,txpin,mode,baudrate;
  rxpin = _RX;
  txpin = _TX;
  mode = _MODE;
  baudrate = _BAUD;
  
  Serial = fdserial_open(rxpin,txpin,mode,baudrate);
  printi("Serial = <%d>\n",Serial); 		
  printi("\nstarting prop echo\n");
  for(;;){
    c = fdserial_rxChar(Serial);
    printi("received <%02x>\n", c); 	
  }    
  return 0;
}

