/**
 * This is the serial communicationd packet.h header file
 * BeagleBone Black implementation
 */

#ifndef _PACKET_H_
#define _PACKET_H_

#include "/home/mam1/Git/Pcon-TNG/Pcon-share/shared.h"

#define _MODEMDEVICE "/dev/ttyO1"	//Beaglebone Black serial port
#define _BAUD 9600
#define _MAX_PACKET	254

#define      False 0
#define      True 1

typedef struct termios _TERMIOS;

/********************** packet frame structures ************************/
typedef struct {
    uint8_t length; // total packet length including length and checksum byte
    uint8_t data[254]; // add 1 for checksum
} _packet;

typedef struct {
  uint8_t       f_type;
  uint8_t       day;
  uint8_t       channel;
  uint8_t       rcnt;
  uint32_t      rec[_MAX_SCHEDULE_RECS];
} _schedule_frame;

typedef struct {
  uint8_t   f_type;
  uint8_t   state;
  uint8_t   mode;
  int     on_time;
  int     off_time;   
} _channel_frame;

typedef struct {
  uint8_t   state;
  uint8_t   mode;
  int     on_time;
  int     off_time;
} _channel_data;  

typedef struct {
  uint8_t     f_type;
  uint8_t     ack_byte;
} _ack_frame;

typedef struct {
  uint8_t         f_type;
  uint8_t         ping;
} _ping_frame;

/*************************  serial io routines *********************************/
void SerialError (int S, _TERMIOS *old); 
int SerialInit(char *device, int bps, _TERMIOS *old, int *port);
uint8_t GetByte(int Port);
uint8_t PutByte(int Port, unsigned char c);
void BuildPkt(uint8_t N, unsigned char *frame, unsigned char *pkt);
void PrintPkt(unsigned char *pkt);
void SndPacket(int Port, unsigned char *pkt);
void waitstart(int port);
uint8_t RcvPacket(int port, uint8_t *pkt);
int WaitAck(int port);

 #endif