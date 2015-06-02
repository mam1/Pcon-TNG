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
#include "shared.h"


//typedef struct _packetruct {
//    uint8_t length; // total packet length including length and checksum byte
//    uint8_t data[PACKET_DLEN+1]; // add 1 for checksum
//} _packet;

typedef struct {
    uint8_t length; // total packet length including length and checksum byte
    uint8_t data[PACKET_DLEN+1]; // add 1 for checksum
} _packet;
/*
typedef struct {
  uint8_t       f_type;
  uint8_t       day;
  uint8_t       channel;
  uint8_t       rcnt;
  uint32_t      rec[_MAX_SCHEDULE_RECS];
} _schedule_frame;

typedef struct {
	uint8_t		f_type;
	uint8_t		state;
	uint8_t		mode;
	int			on_time;
	int 		off_time;		
} _channel_frame;

typedef struct {
	uint8_t		state;
	uint8_t		mode;
	int			on_time;
	int 		off_time;
} _channel_data;  
*/
typedef struct {
  uint8_t     f_type;
  uint8_t     ack_byte;
} _ack_frame;  

void packet_cog(void *parm);
    
void _packetart(fdserial *rec);
void _packetop(void);
    
int packet_make(_packet *pkt, char *s, int len);
int packet_send(text_t *port, _packet *pkt);
int packet_ready(void);
int packet_read(_packet *pkt);
int packet_print(_packet *pkt);

void send_ack(fdserial *port, _ack_frame *f, _packet *p);


#endif