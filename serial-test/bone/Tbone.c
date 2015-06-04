
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>		//sleep
#include <string.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <termios.h>
#include "packet.h"

 /* globals */

uint8_t                   pkt[_MAX_PACKET];
 _ping_frame              ping_frame = {.f_type = _PING_F, .ping = _ACK};

int main(void){
  int                       Serial;     //  port for UART1
  _TERMIOS                  oldtio;     // buffer to hold existing termios settings
  int          c, cc;

  printf("\n*****************************************************************************\n\n");
	printf("start test of sending a serial stream to bone UART1 <%s>\ninitializing UART1\n", _MODEMDEVICE);
	Serial = SerialInit(_MODEMDEVICE,_BAUD,&oldtio,&Serial);
	printf("UART1 initialized, serial handle %i\n",Serial);
  printf("\n  s - send _SOH to UART1\n");
  printf("  h - send packet header to UART1\n");
  printf("  p - send ping packet to UART1\n");
  printf("  r - receive a byte from UART1\n");
  printf("  q - terminate program\n\n> ");
  for(;;){
    while((c = getchar()) == '\n');                   //eat newlines
    switch(c){
      case 'q':
        /* cleanup and terminate */
        // system("/bin/stty cooked");             //switch to buffered input
        // system("/bin/stty echo");               //turn on terminal echo
        if(Serial != -1){                       //if we have a valid handle
          tcsetattr(Serial, TCSANOW, &oldtio);  //restore old tremios settings
          close(Serial);
        }
        printf("\nnormal termination\r\n\n");
        return 0;
        break;
      case 's':
        cc = 'a';
        printf("> sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        cc = 'b';
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
        printf("> sending <%02x> to UART1\n> ",cc);
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
        if(WaitAck(Serial))
            printf("received ack from the prop\n");
        break;
      case 'r':
        printf("receive char from UART1 - ");
        printf("<%02x>\n",GetByte(Serial));
        break;
      default:
        printf("what ?\n> ");
    }
  }

/* cleanup and terminate */
  if(Serial != -1){                       //if we have a valid handle
    tcsetattr(Serial, TCSANOW, &oldtio);  //restore old tremios settings
    close(Serial);
  }
  printf("\n*** abnormal termination\r\n\n");
	return 0;
}