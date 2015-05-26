/**
 * @file packet.c
 * Packet utilities.
 * 
 * 
 */

#include <stdio.h>
#include "packet.h"

static volatile _packet     queue[PACKET_QLEN];
static volatile int         qhead = 0;
static volatile int         qtail = 0;
static volatile int         qcount = 0;

static volatile fdserial *grec;
static char packet_stack[sizeof(_thread_state_t)+20*sizeof(int)];
static int gcog;

void packet_cog(void *parm);

void packet_start(fdserial *rec)
{
    grec = rec;
    gcog = cogstart(packet_cog, 0, packet_stack, sizeof(packet_stack));
    printi("packet cog started, stack %d bytes, running on cog %d\n", sizeof(packet_stack), gcog);
}


void packet_stop(void)
{
    if (gcog > 0) {
        //debug("packet_stop %d\n", gcog);
        cogstop(gcog);
        gcog = 0;
    }    
}
        
void packet_cog(void *parm)
{
    int len = 0;
    int byte = 0;
    int n = 0;
    int sum = 0;
    
    uint8_t c1,c2;
    
    volatile _packet *pkt = 0;
    
    for (;;) { // for (ever)
        /* got a char? */
        if (fdserial_rxReady((fdserial*)grec)) {

            /* look for the start of a packet */
            while ((c1!=_SOH) || (c2!=_STX)) { 
              c1 = c2;
              c2 = fdserial_rxTime((fdserial*)grec,10);
            }
            
            /* read packet length */
            len = fdserial_rxChar((fdserial*)grec);
            if (len > -1) {
                pkt = &queue[qhead];
                pkt->length = len;
                sum = 0;              
                waitcnt(CNT+CNT/100);             // wait for packet
                for (n = 0; n < len; n++) {       // get bytes while valid
                    byte = fdserial_rxTime((fdserial*)grec,10);
                    if (byte < 0) break;          //no data available
                    
                    if (n < len-1) sum += byte;
                    pkt->data[n] = byte;          // last byte is sum
                }
                
                sum  &= 0xff;
                byte &= 0xff;
                  

                // if valid length and sum increment queue head
                // otherwise recycle queue entry.
                if (n > len-2) {
                    if (sum == byte)
                    {
                        qhead++;
                        qhead &= PACKET_QMASK;
                        qcount++;
                    }                    
                }                    
            }               
        }            
    }        
    // don't exit function
}
/************************************
 **       RECEIVE PACKET           **
 ************************************
Byte RcvPacket(Byte *pkt) {
   Byte  Ck1, Ck2, N, i;
  
   waitstart();                          // Wait for start
   N = GetByte();                        // Get the packet size
   Ck1 = 0;                              // Start with checksum at zero
   for (i=0; i<N; i++) {
      *pkt = GetByte();                  // Get next byte, save it in Packet buffer
      Ck1 += *pkt;                       // Accumulate running checksum
      pkt++;                             // next byte
   }
   Ck2 = GetByte();                      // Get the senders checksum
   Ck1 %= 256;                           // Finish the checksum calculation
   if (Ck1 == Ck2) { return N; } else { return 0; }
}
*/



/**************************************************************************************
 **                                                                                  **
 **                   Packet  Routines                                               ** 
 **                   These routines Send or receive a Packet                        **
 **                                                                                  **
 ** -------------------------------------------------------------------------------- **
 **                      Packet Definition                                           **
 **     Each block begins with <soh> <stx> and a <count> followed by the data        **
 **     bytes and the <checksum>.                                                    **
 **                                                                                  **
 **************************************************************************************/
int packet_make(_packet *pkt, uint8_t *data, int len)
{
    int         n = 0;
//    int         sum = 0;
    int        i;
    uint8_t    Ck1, *d;
    
    pkt->length = len+1;          // add a byte for the checksum
    d = data;                     // set pointer to begining of packet data
    Ck1 = 0;                      // clear the checksum

   for (i=0; i<len; i++) {
//       printi("    processsing byte <%2x>\n",*d);
       Ck1 += *d;                     // Accumulate the checksum
       pkt->data[i] = data[i];        // load packet data buffer
       d++;                           // next byte
   }
   Ck1 %= 256; 
   pkt->data[len] = Ck1;  
  
//    printi("check sum 2 %x\n",Ck1);
//    packet_print(pkt);
    pkt->data[len] = Ck1;
//    packet_print(pkt);

    return n;
}

int make_schedule_frame(_packet *pkt,uint8_t *data,int len,int day,int channel,uint32_t *sch){
    int     i,ii;
    _packed N;

    data[0] = _SCHEDULE_F;
    data[1] = (uint8_t)day;
    data[2] = (uint8_t)channel;
    data[3] = (uint8_t)*sch;
    ii = 4;
    for(i = data[3]+1; i > 0; i--){
      N.MyLong = *sch++;
      PackLong(&data[ii], N);
      ii += 4;
    }     
    packet_make(pkt,data,ii);
    return 0;
  }    


int packet_send(fdserial *port, _packet *pkt)
{
    int n;
    //packet_print(pkt);
    
    writeChar(port, _SOH);              // Send start of packet
    writeChar(port, _STX);              // Send start of packet   
    writeChar(port, pkt->length);       // Send packet size
    
    /* send packet */
    for (n = 0; n < pkt->length; n++) {
        writeChar(port, pkt->data[n]);
//        printi(" sending <%2x>\n", pkt->data[n]);
    }
    return 0;        
}

int packet_ready(void)
{
    int rc = 0;
    if (qhead != qtail) rc = 1;
    return rc; 
}


int packet_read(_packet *rxpkt)
{
    _packet *pkt = (_packet*) &queue[qtail];
    memcpy((void*)rxpkt, (void*)pkt, sizeof(_packet));
    qtail++;
    qtail &= PACKET_QMASK;
    return pkt->length;
}


int packet_print(_packet *pkt)
{
    int n;
    int len = pkt->length;
    printi("packet len %d : <", len);
    for (n = 0; n < len; n++) {
        printi(" %02x", pkt->data[n]);
    }
    printi(">\n");
    return 0;
}

int schedule_frame_print(_schedule_frame *f){
  int             i;
  printi("schedule frame:\n");
  printi("  type=%d, day=%d, channel=%d, schedule records = %d\n", f->f_type,f->day,f->channel,f->rcnt);
  printi("  schedule record count plus schedule records 0 - %d:\n",f->rcnt);
  for(i=0;i<f->rcnt+1;i++)
    printi("    <%04x>\n",f->rec[i]);  
  
  return 0;
}  

/********************************************************/
/**              Pack / UnPack  Routines               **/
/********************************************************/
 
/*  Insert / remove propeller long from the Packet    */ 
void PackLong(uint8_t *p, _packed N) { // N - 4 byte long,   p - insertion point
    *p = N.MyByte[0];  p++;
    *p = N.MyByte[1];  p++;
    *p = N.MyByte[2];  p++;
    *p = N.MyByte[3];  p++;
}

uint32_t UnPackLong(uint8_t *p) {   // p pointer to start of 4 byte long
    _packed N; 
    N.MyByte[0] = *p;  p++;
    N.MyByte[1] = *p;  p++;
    N.MyByte[2] = *p;  p++;
    N.MyByte[3] = *p;  p++;
    return N.MyLong;
}

int pack_schedule_frame(_schedule_frame *sf_ptr){

  return 0;
}

int unpack_schedule_frame(uint8_t *byte_ptr, _schedule_frame *sf_ptr){
    int         i;
    
    sf_ptr->f_type = *byte_ptr++;
    sf_ptr->day = *byte_ptr++;
    sf_ptr->channel = *byte_ptr++;
    sf_ptr->rcnt =  *byte_ptr++;
    
    for(i=0;i<sf_ptr->rcnt+1;i++){
      sf_ptr->rec[i] = UnPackLong(byte_ptr);
      byte_ptr += 4;              
    }               
    return 0;
}  

/************************************************************/

int send_ack(_packet *pkt, fdserial *pktport, _ack_frame *ack_ptr){
  packet_make(pkt,(uint8_t *)ack_ptr,sizeof(*ack_ptr));
  packet_send(pktport,pkt);
  return 0;
} 

int send_PING(_packet *pkt,fdserial *pktport,uint8_t *ping_data){
  /*
  int       i;
  pkt->length = 1;
  
  for(i=0;i<size 
  pkt->data[0] = 1;
  pkt->data[1] = _Ping;
  packet_send(pktport,&g_packet); 
  */
  
  return 0;
  }    
