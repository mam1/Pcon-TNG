/**
 * @file packet.h
 */


#ifndef __PACKET_H__
#define __PACKET_H__


#include <termios.h>

// typedef struct termios _TERMIOS; 

typedef struct {
  uint8_t         f_type;
  uint8_t         ping;
} _ping_frame;

void SerialClose(int port, struct termios *old);
void SerialError (int port, struct termios *old);
int SerialInit(char *device, int bps, struct termios *old);
uint8_t GetByte(int Port, struct termios *old);
uint8_t PutByte(int Port, unsigned char c);
void BuildPkt(uint8_t N, unsigned char *frame, unsigned char *pkt);
void SndPacket(int Port, unsigned char *pkt );
uint8_t RcvPacket(int port, uint8_t *pkt, struct termios *old);
void waitstart(int port, struct termios *old);
int WaitAck(int port,uint8_t *pac, struct termios *old);
void PrintPkt(unsigned char *pkt);

#endif