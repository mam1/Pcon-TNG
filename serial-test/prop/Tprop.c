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
fdserial 			      *Serial;
_packet             pkt;
_ack_frame          ack_frame = {.f_type = _ACK_F, .ack_byte = _ACK};
//_schedule_frame    schedule_frame;  

int main(void)
{
  int             i, c; 
  Serial = fdserial_open(_RX,_TX,_MODE,_BAUD);
  printi("serial port opened\n"); 
  _packetart(Serial);
  printi("starting cog to monitor serial connection to the BeagleBone\n"); 	
  printi("\n*************************************\n\n");
  for(;;){
    if (packet_ready()) {
        packet_print(&pkt);
        packet_read(&pkt);
        printi("\nPacket dequeued");
        packet_print(&pkt);
        i = 0;
        switch(pkt.data[i++]){
          case _PING_F:
            printi("received a ping frame \n");
            send_ack(Serial,&ack_frame,&pkt);
            printi("sent an ack to bone\n");           
            break;
          case _SCHEDULE_F:
            printi("received a schedule frame \n");
            send_ack(Serial,&ack_frame,&pkt);
            printi("sent an ack to bone\n");           
            break;
          default: 
            printi("*** error *** unknown packet type\n");
          }            
    }                  	
  }    
  return 0;
}

