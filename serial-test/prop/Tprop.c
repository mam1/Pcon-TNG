                                                  /**
 * This is the main Tprop program file.
 */

#include <stdio.h>
#include <simpletools.h>                    // Library include
#include <fdserial.h>
#include "shared.h"
#include "packet.h"


#define _RX 1
#define _TX 0
#define _MODE 0
#define _BAUD 9600

/*  globals */
fdserial 			*Serial;
packet_st     pkt; 

int main(void)
{
  int             i, c; 
  Serial = fdserial_open(_RX,_TX,_MODE,_BAUD);
  printi("serial port opened\n"); 
  packet_start(Serial);
  printi("	serial 	
  printi("\nstarting prop echo\n");
  for(;;){
    if (packet_ready()) {
        packet_read(&pkt);
        printi("Packet returned from read\n");
        packet_print(&pkt);
        i = 0;
        switch(pkt.data[i++]){
          case _PING_F:
            printi("received a ping frame \n");
            break;
          default: 
            printi("*** error *** unknown packet type\n");
          }            
    }                  	
  }    
  return 0;
}

