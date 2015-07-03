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
#define _BAUD 115200

/*  globals */
fdserial 			      *Serial;
_packet             pkt;
_ack_frame          ack_frame = {.f_type = _ACK_F, .ack_byte = _ACK};
_nack_frame         nack_frame = {.f_type = _NACK_F, .nack_byte = _NACK};
_packet             ack_packet;
_packet             nack_packet;


//_schedule_frame    schedule_frame;  

int main(void)
{
  int             i;
  
  /* build reusable ack and nack packets */
  packet_make(&ack_packet, (char *)&ack_frame, sizeof(ack_frame));
  packet_make(&nack_packet, (char *)&nack_frame, sizeof(nack_frame));
  /* open a fullduplex serial connection to the UART1 on the bone */
  Serial = fdserial_open(_RX,_TX,_MODE,_BAUD);
  printi("serial port opened\n"); 
  _packetart(Serial); // start cog to monitor serial connection
  printi("starting cog to monitor serial connection to the BeagleBone\n"); 	
  printi("\n*************************************\n\n");
  for(;;){
    if (packet_ready()) {
        packet_read(&pkt);
        printi("Packet dequeued\n  ");
        packet_print(&pkt);
        i = 0;
        switch(pkt.data[i++]){
          case _PING_F:
            printi("  received a ping frame \n");
            packet_send(Serial,&ack_packet);        
            break;
          case _SCHEDULE_F:
            printi("  received a schedule frame \n");
//            if(marshal_schedule(&pkt.data[i],))
              packet_send(Serial,&ack_packet);
//            else 
//              packet_send(Serial,&nack_packet);                   
            break;
          default: 
            printi("  unknown packet type\n");
            packet_send(Serial,&nack_packet);        
          }            
    }                  	
  }    
  return 0;
}

