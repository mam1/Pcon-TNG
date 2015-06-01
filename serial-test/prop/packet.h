/**
 * @file packet.h
 */


#ifndef __PACKET_H__
#define __PACKET_H__


#define CONS_RX 0
#define CONS_TX 1
#define CONS_MODE 0
#define CONS_BAUD 115200


#define PROD_RX 1
#define PROD_TX 0
#define PROD_MODE 0
#define PROD_BAUD CONS_BAUD

#define PACKET_DLEN 100

#define PACKET_QLEN 16 // power of 2 for queue
#define PACKET_QMASK (PACKET_QLEN-1)

#include <stdint.h>
#include "simpletext.h"
#include "fdserial.h"


typedef struct packet_struct {
    uint8_t length; // total packet length including length and checksum byte
    uint8_t data[PACKET_DLEN+1]; // add 1 for checksum
} packet_st;

void packet_cog(void *parm);
    
void packet_start(fdserial *rec);
void packet_stop(void);
    
int packet_make(packet_st *pkt, char *s, int len);
int packet_send(text_t *port, packet_st *pkt);
int packet_ready(void);
int packet_read(packet_st *pkt);
int packet_print(packet_st *pkt);


#endif