/**
 * @file packet.h
 */


#ifndef __PACKET_H__
#define __PACKET_H__


#include <termios.h>
#include "../Pcon-share/typedefs.h"

// typedef struct termios _TERMIOS; 


void 	SerialClose(int port, struct termios *old);
void 	SerialError (int port, struct termios *old);
int 	SerialInit(char *device, int bps, struct termios *old);
uint8_t GetByte(int Port, struct termios *old,int *error_code);
uint8_t PutByte(int Port, unsigned char c);
void 	BuildPkt(uint8_t N, unsigned char *frame, unsigned char *pkt);
void 	SndPacket(int Port, unsigned char *pkt, struct termios *old );
int 	Snd_P(int Port, unsigned char *pkt, struct termios *old  );
uint8_t RcvPacket(int port, uint8_t *pkt, struct termios *old);
int 	waitstart(int port, struct termios *old);
int 	WaitAck(int port,uint8_t *pac, struct termios *old);
void 	PrintPkt(unsigned char *pkt);
int 	make_schedule_frame(uint8_t *pkt,uint8_t *data,int len,int day,int channel,uint32_t *sch);
void 	PackLong(uint8_t *p, _packed N);  // N - 4 byte long,   p - insertion point
int 	UnPackLong(uint8_t *p);    // p pointer to start of 4 byte long
 

#endif