                                                  /**
 * This is the main program file for the prop
 */

#include <stdio.h>
#include <simpletools.h>                    // Library include
#include <fdserial.h>
#include "shared.h"
#include "typedefs.h"
#include "packet.h"



 #define _RX 1
 #define _TX 0
 #define _MODE 0
// //#define _BAUD 14400

// #define      _NACK            0x15
// #define      _NACK_F         	6 //


/*  globals */
fdserial 			      *Serial;
_packet             pkt;
_ack_frame          ack_frame = {.f_type = _ACK_F, .ack_byte = _ACK};
_nack_frame         nack_frame = {.f_type = _NACK_F, .ack_byte = _NACK};
_schedule_frame     schedule_frame;

_ack_packet         ack_packet = {
                      .hb1 = _SOH, 
                      .hb2 = _STX, 
                      .length = 3, 
                      .f_type = _ACK_F, 
                      .f_b1 = _ACK};    

uint32_t            sch[_DAYS_PER_WEEK * _NUMBER_OF_CHANNELS * _SCHEDULE_SIZE]; 

uint8_t checksum(_packet *pkt){
   int  i,len;
   uint8_t      *byte_ptr;
   uint8_t Ck1;
   
   byte_ptr = (uint8_t)&pkt->length;
 //  byte_ptr = (uint8_t)pkt;

   len = pkt->length;
   printi("--- packet length %d\n",len);
   Ck1 = 0;                          // Clear the checksum
   for (i=0; i<len-1; i++) {
       Ck1 += *byte_ptr;                  // Accumulate the checksum
       byte_ptr++;                        // next byte
   }
   Ck1 %= 256;
   printi("--- checksum <%02x>\n",Ck1);
  return  Ck1;
} 

int main(void)
{
  uint8_t           *start_packet, *packet_header;
//  int             i; 
  printi("\n\nPacket Pounder");
  Serial = fdserial_open(_RX,_TX,_MODE,9600);
  printi("serial port opened\n"); 
  _packetart(Serial);
  printi("cog to monitor serial connection to the BeagleBone started\n"); 	
  printi("\n*************************************\n\n");
  
  /* load checksums */

  packet_header = start_packet = (uint8_t *)&ack_packet;
  start_packet++;
  start_packet++;
  printi("start_packet <%02x>\n",*start_packet);
  ack_packet.checksum = checksum(start_packet);
  packet_print(start_packet);



  for(;;){
    if (packet_ready()) {
        packet_read(&pkt);
        packet_send(Serial, (_packet *)&ack_packet.length);            
    }                  	
  }    
  return 0;
}

