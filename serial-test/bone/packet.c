/**
 * This is the serial communicationd packet.c program file
 * BeagleBone Black implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>		//sleep
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include "packet.h"

 /* globals */

uint8_t                   RcvPkt[_MAX_PACKET];

 /*************************  serial io routines *********************************/
void SerialError (int S, _TERMIOS *old){
    system("/bin/stty cooked");     	//switch to buffered input
    system("/bin/stty echo");     		//turn on terminal echo
    if(S != -1){						          //if we have a valid handle
      tcsetattr(S, TCSANOW, old);     //restore old tremios settings
      close(S);
    }
    printf("*** application terminated by SerialError\r\n\n");
    exit(-1);
  }

/* initialize UART */  
int SerialInit(char *device, int bps, _TERMIOS *old,int *port) {
    int               		ret;
     _TERMIOS                newtio;
	/* Load the pin configuration */
    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      *port = -1;
      SerialError(-1,old);
    }
	/* open UART1 */
        // fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY  | O_NDELAY);
    *port = open(device, O_RDWR | O_NOCTTY); //Open device in read/write mode
    if (*port == -1) {
      printf("Error - Unable to open '%s'.\n", device);
      exit(1); 
    }
    ret = tcgetattr(*port, old);            //save old tremios settings
    if (ret < 0) {
      printf("*** problem saving old tremios settings\r\n");
      SerialError(*port,old);
    }
    bzero(&newtio, sizeof(newtio)); // clear struct for new port settings 
    tcgetattr(*port, &newtio);
    newtio.c_cflag = bps | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(*port, TCIFLUSH);
    tcsetattr(*port, TCSANOW, &newtio);

    return *port;     
}

uint8_t GetByte(int Port){
  char x;  
  read (Port, &x, sizeof(x) ); 
  return x; 
}

uint8_t PutByte(int Port, unsigned char c){
  write(Port, &c, sizeof(c));
  return -1;
}

void BuildPkt(uint8_t N, unsigned char *frame, unsigned char *pkt) {
   uint8_t i;
   *pkt++ = N+1;
   for (i=0; i<N; i++) {
    *pkt++ = *frame++; 
   }
   return;
}

void PrintPkt(unsigned char *pkt){
  int         i,plen;
  plen = *pkt;
  printf("packet length %i  ",plen);
  for(i=0; i<plen; i++) printf("<%02x>", *pkt++);
  printf("\n");
  return;
}

void SndPacket(int Port, unsigned char *pkt ) {
   int  i,len;
   uint8_t Ck1;
   len = *pkt;
   PutByte(Port,_SOH);               // Send start of packet
   PutByte(Port,_STX);               // Send start of packet
   PutByte(Port,*pkt++);             // Send packet size   
   Ck1 = 0;                          // Clear the checksum
   for (i=0; i<len-1; i++) {
       PutByte(Port,*pkt);           // Send the next data character 
       Ck1 += *pkt;                  // Accumulate the checksum
       pkt++;                        // next byte
   }
   Ck1 %= 256;
   *pkt = Ck1;
   PutByte(Port, Ck1 );               // Send the checksum
}

void waitstart(int port) {             // This waits for soh and then stx in that order
  uint8_t c1,c2=0;
  while ((c1!=_SOH) || (c2!=_STX)){
    c1 = c2; 
    c2 = GetByte(port);
  }
  return; 
}

uint8_t RcvPacket(int port, uint8_t *pkt) {
   uint8_t  Ck1, Ck2, N, i;
  
   waitstart(port);                          // Wait for start
//   printf("got a packet header\n");
   N = GetByte(port);                        // Get the packet size
//   printf("packet size = %i\n", N);
   Ck1 = 0;                              // Start with checksum at zero
   for (i=0; i<N-1; i++) {
      *pkt = GetByte(port);                  // Get next byte, save it in Packet buffer
//      printf("got byte <%02x>\n", *pkt);
      Ck1 += *pkt;                       // Accumulate running checksum
      pkt++;                             // next byte
   }
   Ck2 = GetByte(port);                      // Get the senders checksum

//   printf("Ck1 - calculated chcksum = %i, Ck2 - checksum from packet = %i\n", Ck1, Ck2);  
                            // Finish the checksum calculation
   Ck1 &= 0xff;
   Ck1 %= 256;

//   printf("Ck1 - calculated chcksum = %i, Ck2 - checksum from packet = %i\n", Ck1, Ck2);                           // Finish the checksum calculation
   if (Ck1 == Ck2) { return N; } else { return 0; }
}


int WaitAck(int port) {
    uint8_t PSize;

    PSize = RcvPacket(port, RcvPkt);                    // Receive Packet
//    printf("back from RcvPacket\n");
    if ((PSize == 0) || (RcvPkt[0]==_NAK)) {
       printf("Cmd Failed\n> ");
       return False;
    }
    return True;
}

