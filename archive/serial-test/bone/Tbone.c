
#include <stdio.h>
#include <stdlib.h>   //system calls
//#include <strings.h>
#include <unistd.h>		//sleep
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
// #include <sys/ioctl.h>
#include "../shared/shared.h"
#include "packet.h"



/* typedefs */
// typedef struct termios struct termios;

// typedef struct {
//     uint8_t length; // total packet length including length and checksum byte
//     uint8_t data[254]; // add 1 for checksum
// } _packet;

// typedef struct {
//   uint8_t       f_type;
//   uint8_t       day;
//   uint8_t       channel;
//   uint8_t       rcnt;
//   uint32_t      rec[_MAX_SCHEDULE_RECS];
// } _schedule_frame;

// typedef struct {
//   uint8_t   f_type;
//   uint8_t   state;
//   uint8_t   mode;
//   int     on_time;
//   int     off_time;   
// } _channel_frame;

// typedef struct {
//   uint8_t   state;
//   uint8_t   mode;
//   int     on_time;
//   int     off_time;
// } _channel_data;  

// typedef struct {
//   uint8_t     f_type;
//   uint8_t     ack_byte;
// } _ack_frame;

// typedef struct {
//   uint8_t         f_type;
//   uint8_t         ping;
// } _ping_frame;

/* globals */
struct termios                  oldtio;
struct termios                  newtio;
int                       Serial = -1;
uint8_t                   pkt[_MAX_PACKET];
uint8_t                   SndPkt[_MAX_PACKET];
uint8_t                   RcvPkt[_MAX_PACKET];
 _ping_frame              ping_frame = {.f_type = _PING_F, .ping = _ACK};

int main(void){

  int          c, cc;
  uint8_t       *p;

  p = pkt;
  *p++ = 2;
  *p++ = _PING_F;
  *p++ = _PING;

  printf("\n*****************************************************************************\n\n");
	printf("start test of sending a serial stream to bone UART1 <%s>\ninitializing UART1\n", _MODEMDEVICE);
	Serial = SerialInit(_MODEMDEVICE,B9600,&oldtio);
	printf("UART1 initialized\n");
  printf("\n  s - send _SOH to UART1\n");
  printf("  h - send packet header to UART1\n");
  printf("  p - send ping packet to UART1\n");
  printf("  r - recieve a byte from UART1\n");
  printf("  q - terminate program\n\n> ");
  for(;;){
    while((c = getchar()) == '\n');                   //eat newlines
    switch(c){
      case 'q':
        SerialClose(Serial, &oldtio);
        printf("\nnormal termination\r\n\n");
        return 0;
        break;
      case 's':
        cc = 255;
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = 255;
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = 'c';
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = 'd';
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = 'e';
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        break;
      case 'h':
        cc = _SOH;
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = _STX;
        printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        break;
      case 'p':
        printf("build packet from ping frame\n");
        BuildPkt(sizeof(ping_frame), (uint8_t *)&ping_frame, pkt);
        PrintPkt(pkt);
        SndPacket(Serial, pkt);
        printf("ping frame sent to UART1\n> ");
        if(WaitAck(Serial,RcvPkt))
            printf("received ack from the prop\n> ");
        break;
      case 'r':
        printf("receive char from UART1 - ");
        printf("<%02x>\n",GetByte(Serial));
        break;
      default:
        printf("what ?\n> ");
    }
  }

/* cleanup and teriminate */
  // system("/bin/stty cooked");             //switch to buffered input
  // system("/bin/stty echo");               //turn on terminal echo
  if(Serial != -1){                       //if we have a valid handle
    tcsetattr(Serial, TCSANOW, &oldtio);  //restore old tremios settings
    close(Serial);
  }
  printf("\n*** abnormal termination\r\n\n");
	return 0;
}