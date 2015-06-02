/**
 * @file packet.c
 * Packet utilities.
 */


#include <stdio.h>
#include "packet.h"
#include "shared.h"

static volatile packet_st queue[PACKET_QLEN];
static volatile int qhead = 0;
static volatile int qtail = 0;
static volatile int qcount = 0;

static volatile fdserial *grec;
static char packet_stack[sizeof(_thread_state_t)+20*sizeof(int)];

static int gcog;

void packet_start(fdserial *rec)
{
    grec = rec;
    gcog = cogstart(packet_cog, 0, packet_stack, sizeof(packet_stack)); 
    //debug("packet_start %d cog %d\n", sizeof(packet_stack), gcog);
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
    int         len = 0;
    int         byte = 0;
    int         n = 0;
    int         sum = 0;

    uint8_t     c1, c2;     
    
    volatile packet_st *pkt = 0;
    
    for (;;) { // for (ever)
        // got a char?
        if (fdserial_rxReady((fdserial*)grec)) {

            // wait for a packet header
            while ((c1!=_SOH) || (c2!=_STX)) { 
              c1 = c2;
              c2 = fdserial_rxTime((fdserial*)grec,10);
            };

            // read length
            len = fdserial_rxChar((fdserial*)grec);
            if (len > -1) {
                //OUTA |= LED2;
                // read into packet packet
                pkt = &queue[qhead];
                pkt->length = len;
                sum = 0;
  
                // wait for packet              
                waitcnt(CNT+CNT/100);
                // get bytes while valid
                for (n = 0; n < len; n++) {
                    byte = fdserial_rxTime((fdserial*)grec,10);
                    //byte = fdserial_rxCheck((fdserial*)grec);
                    if (byte < 0) break;
                    if (n < len-1) sum += byte;
                    pkt->data[n] = byte; // last byte is sum
                }
                
                sum  &= 0xff;
                byte &= 0xff;
                
                // if valid length and sum increment queue head
                // otherwise recycle queue entry.
                if (n > len-2) {
                    if (sum == byte)
                    {
                        //OUTA &= ~LED2;
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


int packet_make(packet_st *pkt, char *s, int len)
{
    int n = 0;
    int sum = 0;
    
    pkt->length = len+1;
    printi("packet length set by packet_make to %d\n",pkt->length);
    
    for (n = 0; n < len && n < PACKET_DLEN; n++) {
        pkt->data[n] = s[n];
        if (n < len-1) sum += s[n];
    }        
    sum &= 0xff;
    pkt->data[n] = sum;
    //packet_print(pkt);
    return n;
}


int packet_send(fdserial *port, packet_st *pkt)
{
    int n;
    packet_print(pkt);
    writeChar(port, pkt->length);
    for (n = 0; n < pkt->length; n++) {
        writeChar(port, pkt->data[n]);
    }
    return 0;        
}


int packet_ready(void)
{
    int rc = 0;
    if (qhead != qtail) rc = 1;
    return rc; 
}


int packet_read(packet_st *rxpkt)
{
    packet_st *pkt = (packet_st*) &queue[qtail];
    memcpy((void*)rxpkt, (void*)pkt, sizeof(packet_st));
    qtail++;
    qtail &= PACKET_QMASK;
    return pkt->length;
}


int packet_print(packet_st *pkt)
{
    int n;
    int len = pkt->length;
    printi("\npacket len %d : ", len);
    for (n = 0; n < len; n++) {
        printi(" %02x", pkt->data[n]);
    }
    printi("\n");
    return 0;
}
