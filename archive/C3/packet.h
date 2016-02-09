/**
 * @file packet.h
 */
#include "typedefs.h"
#include "shared.h"

#ifndef __PACKET_H__
#define __PACKET_H__


#define CONS_RX 0
#define CONS_TX 1
#define CONS_MODE 0
#define CONS_BAUD _BAUD


#define PROD_RX 1
#define PROD_TX 0
#define PROD_MODE 0
#define PROD_BAUD CONS_BAUD

#define PACKET_DLEN _MAX_PACKET

#define PACKET_QLEN 16 // power of 2 for queue
#define PACKET_QMASK (PACKET_QLEN-1)

#include <stdint.h>
#include "simpletext.h"
#include "fdserial.h"
#include "shared.h"
#include "typedefs.h"

void packet_cog(void *parm);
    
void _packetart(fdserial *rec);
void _packetop(void);
    
int packet_make(_packet *pkt, uint8_t *frame, int len);
int packet_send(text_t *port, _packet *pkt);
int packet_ready(void);
int packet_read(_packet *pkt);
int packet_print(_packet *pkt);

void send_ack(fdserial *port, _ack_frame *f, _packet *p);
void send_nack(fdserial *port, _nack_frame *f, _packet *p);
int marshal_schedule(uint8_t *f,uint32_t *);
void PackLong( uint8_t* p, _packed N );
int UnPackLong( uint8_t* p );


#endif