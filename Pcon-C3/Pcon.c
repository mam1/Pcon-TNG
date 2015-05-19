/*
 * Pcon.c
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */

#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <propeller.h>
#include "Pcon.h"
#include "simpletext.h"
#include "fdserial.h"
#include "packet.h"
#include "schedule.h"

fdserial          *pktport;
_packet           g_packet,r_packet;
_schedule_frame   s_frame, s_frame_read; 
uint32_t          sch[_SCHEDULE_SIZE];
uint32_t          w_sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE];

/***************** global code to text conversion ********************/
const char *day_names_long[7] = {
     "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
const char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char *onoff[2] = {"off"," on"};
const char *con_mode[3] = {"manual","  time","time & sensor"};
const char *sch_mode[2] = {"day","week"};
const char *c_mode[4] = {"manual","  time","   t&s"," cycle"};
  
int main(int argc, char *argv[])
{
  uint8_t         *byte_ptr;
  int               i;
  
  sleep(1);
  //printf("\033\143"); //clear the terminal screen, preserve the scroll back
  	printi("\n*** Pcon  %d.%d.%d ***\n\n",_major_version,_minor_version,_minor_revision);

  pktport = fdserial_open(PROD_RX, PROD_TX, PROD_MODE, PROD_BAUD);
  if(pktport == 0){
    printi("*** packet port open error\n");
    return 1;
  }    
  
  sch[0] = 4;
  sch[1] = 1000877;
  sch[2] = 128274849;
  sch[3] = 183838533;
  sch[4] = 111111111;

  
  printi("packet port opened\n");
  printi("starting packet processing cog ..... ");
  packet_start(pktport);
//  printi("g_packet size = %d\n",sizeof(g_packet));
//  printi("s_frame size = %d\n",sizeof(s_frame));
//  printi("size of header = %d\n", 4);
//  printi("size of schedule = %d\n",4+(sch[0]*4));
//  printi("number of schedule records = %d\n",sch[0]);
//  printi("packet length = %d\n",sch[0]*4+8);
  
  if(make_schedule_frame(&g_packet,(uint8_t *)&s_frame,sizeof(s_frame),1,1,sch)){
    printi("*** make_schedule_frame error\n");
    return 1;
  }
  packet_send(pktport,&g_packet);

  sch[0] = 3;
  sch[1] = 99999999;
  sch[2] = 88888888;
  sch[3] = 77777777; 
    
  if(make_schedule_frame(&g_packet,(uint8_t *)&s_frame,sizeof(s_frame),1,2,sch)){
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
          case _SCHEDULE_F:                       // schedule frame
            printi("recieved a schedule frame\n");
            unpack_schedule_frame(byte_ptr,&s_frame_read);
            disp_all_schedules((uint32_t *)w_sch);
            load_schedule((uint32_t *)w_sch, &s_frame_read.rec[0], s_frame_read.day, s_frame_read.channel);  	//(schedule data, template, day, channel)
            disp_all_schedules((uint32_t *)w_sch);
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

