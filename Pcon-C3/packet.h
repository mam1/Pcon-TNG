/**
 * @file packet.h
 */

#ifndef __PACKET_H__
#define __PACKET_H__


/* Ascii control Characters */
#define      _SOH  0x01
#define      _STX  0x02
#define      etx  0x03
#define      eot  0x04
#define      ack  0x06
#define      nak  0x15
#define      syn  0x16

/* frame types */
#define _SCHEDULE_F   	1	// replace a working schedule with schedule in frame, send ack to sender
#define _CHANNEL_F    	2 	// replace channel data with channel data in frame, send channel data back to sender
#define _TIME_F       	3 	// set real time clock to the time/date in frame, send ack to sender
#define _PING_F			4 	// request for ping, send ping data in frame back to sender	
#define _ACK_F         5 //
#define _REBOOT_F      6 // reboot the C3, program load from EEPROM 

#define CONS_RX 0
#define CONS_TX 1
#define CONS_MODE 0
#define CONS_BAUD 9600

#define PROD_RX 1
#define PROD_TX 0
#define PROD_MODE 0
#define PROD_BAUD CONS_BAUD

#define PACKET_DLEN 128
#define PACKET_QLEN 16 // power of 2 for queue
#define PACKET_QMASK (PACKET_QLEN-1)

#include <stdint.h>
#include "simpletext.h"
#include "fdserial.h"
#include "packet.h"
#include "shared.h"


typedef struct _packetruct {
    uint8_t length; // total packet length including length and checksum byte
    uint8_t data[PACKET_DLEN+1]; // add 1 for checksum
} _packet;

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

typedef struct {
  uint8_t     f_type;
  uint8_t     ack_byte;
} _ack_frame;  

typedef union {uint32_t MyLong; uint8_t MyByte[4]; } _packed;
    
//void _packetart(fdserial *rec);
//void _packetop(void);

void packet_start(fdserial *rec);
void packet_stop(void);
    
int packet_make(_packet *pkt, uint8_t *data, int len);
int make_schedule_frame(_packet *pkt,uint8_t *data,int len,int day,int channel,uint32_t *sch);
int packet_send(text_t *port, _packet *pkt);
int packet_ready(void);
int packet_read(_packet *pkt);
int packet_print(_packet *pkt);
int schedule_frame_print(_schedule_frame *f);

void PackLong(uint8_t *p, _packed N);
uint32_t UnPackLong(uint8_t *p);
int pack_schedule_frame(_schedule_frame *);
int unpack_schedule_frame(uint8_t *byte_ptr, _schedule_frame *sf_ptr);

int send_ack(_packet *pkt, fdserial *pktport, _ack_frame *ack_ptr);
  
  
#endif