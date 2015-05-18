/**
 * This is the main comtest2 program file.
 */


//#include <string.h>
#include "simpletext.h"
#include "fdserial.h"
#include "packet.h"
#include <stdlib.h>
//#include <strings.h>
#include <unistd.h>

fdserial          *pktport;
_packet           g_packet,r_packet;
_schedule_frame   s_frame, s_frame_read; 


uint32_t          sch[_SCHEDULE_SIZE];
  

int main(void)
{
  uint8_t         *byte_ptr;
  int               i;
  
  printi("\n***********************\ncomtest 2.0\n***********************\n\n");
  pktport = fdserial_open(PROD_RX, PROD_TX, PROD_MODE, PROD_BAUD);
  if(pktport == 0){
    printi("*** packet port open error\n");
    return 1;
  }    
  
  sch[0] = 3;
  sch[1] = 1000877;
  sch[2] = 128274849;
  sch[3] = 183838533;
  
  printi("packet port opened\n");
  printi("starting packet processing cog ..... ");
  packet_start(pktport);
//  printi("g_packet size = %d\n",sizeof(g_packet));
//  printi("s_frame size = %d\n",sizeof(s_frame));
//  printi("size of header = %d\n", 4);
//  printi("size of schedule = %d\n",4+(sch[0]*4));
//  printi("number of schedule records = %d\n",sch[0]);
//  printi("packet length = %d\n",sch[0]*4+8);
  
  if(make_schedule_frame(&g_packet,(uint8_t *)&s_frame,sizeof(s_frame),6,7,sch)){
    printi("*** make_schedule_frame error\n");
    return 1;
  }
  packet_send(pktport,&g_packet);

  sch[0] = 3;
  sch[1] = 99999999;
  sch[2] = 88888888;
  sch[3] = 77777777; 
    
  if(make_schedule_frame(&g_packet,(uint8_t *)&s_frame,sizeof(s_frame),6,7,sch)){
    printi("*** make_schedule_frame error\n");
    return 1;
  }
  packet_send(pktport,&g_packet);
  printi("loop waiting for a packet to appear on queue\n");    
  printi("******************************************************\n");
  for(;;){
      printi("checking for packets\n");
      if(packet_ready()){                         // see if there is a packet ready
        printi(">>>> packet ready\n");
        packet_read(&r_packet);                   // dequeue a packet
        byte_ptr = (uint8_t *)&r_packet;
        byte_ptr++;                               // set pointer to start of packet data
        switch(*byte_ptr)                         // unpack frame based on frame type
        {
          case _SCHEDULE_P:                       // schedule frame
            printi("recieved a schedule frame\n");
            unpack_schedule_frame(byte_ptr,&s_frame_read);
            schedule_frame_print(&s_frame_read);
            break;
          default:
            printi("*** unknown packet type <%2x>\n",*byte_ptr); 
        }                  
//        packet_print(&r_packet); 
      }
      sleep(1);
//      packet_send(pktport,&g_packet);
  }      
   
  return 0;
}

