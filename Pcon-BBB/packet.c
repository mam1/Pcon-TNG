/**
 * @file packet.c
 * Packet utilities.
 */

#include <stdio.h>
#include <stdlib.h>   //system calls
#include <strings.h>
#include <unistd.h>		//sleep
#include <termios.h>
#include <sys/fcntl.h>	//file io
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include "packet.h"
#include "../Pcon-share/shared.h"
#include "../Pcon-share/typedefs.h"

/**************************** globals ******************************************/
fd_set                set;
struct timeval        timeout;

/*************************  serial io routines *********************************/

void SerialClose(int port, struct termios *old){
  /* cleanup and terminate */
  system("/bin/stty cooked");             //switch to buffered input
  system("/bin/stty echo");               //turn on terminal echo
  if(port != -1){                         //if we have a valid handle
    tcsetattr(port, TCSANOW, old);        //restore old tremios settings
    close(port);
  }
  return;
}

void SerialError (int port, struct termios *old){
    printf("*** problem communicating with the C3\n");
    SerialClose(port, old);
    printf("*** application terminated\r\n\n");
    exit(-1);
}

/* initialize UART */  
int SerialInit(char *device, int bps, struct termios *old) {
    int                   ret;    //value returned from system call
    int                   sport;  //serial port
    struct termios        newtio; //buffer for new terminal settings

	/* Load the pin configuration */
    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      sport = -1;
      SerialError(-1,old);
    }
	/* open UART1 */
    sport = open(device, O_RDWR | O_NOCTTY); //Open device in read/write mode
    if (sport == -1) {
      printf("Error - Unable to open '%s'.\n", device);
      exit(1); 
    }
    FD_ZERO(&set);                    // clear the set 
    FD_SET(sport, &set);              // add file descriptor to the set 
    timeout.tv_sec = _TIMEOUT_SEC;    // set read time out
    timeout.tv_usec = _TIMEOUT_USEC;
    ret = tcgetattr(sport, old);       // save old tremios settings
    if (ret < 0) {
      printf("*** problem saving old tremios settings\r\n");
      SerialError(sport,old);
    }
    bzero(&newtio, sizeof(newtio)); // clear struct for new port settings 
    tcgetattr(sport, &newtio);
    newtio.c_cflag = bps | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(sport, TCIFLUSH);
    tcsetattr(sport, TCSANOW, &newtio);

    return sport;     
}

uint8_t GetByte(int Port,struct termios *old){
    char                  x;
    int                   rv;

    rv = select(Port + 1, &set, NULL, NULL, &timeout);
    // printf("/n    rv = %i\n\r", rv);
    if(rv == -1){
      perror("select"); /* an error accured */
      SerialError(Port, old);
    }
    else if(rv == 0){
      printf("\n*** read timeout\n\r"); /* a timeout occured */
      rv = select(Port + 1, &set, NULL, NULL, &timeout);
      if(rv == 0){
        printf("\n*** read timeout\n\r"); /* a timeout occured */
        SerialError(Port, old);
      }
      if(rv == -1){
        perror("select"); /* an error accured */
        SerialError(Port, old);
      }
      read(Port, &x, sizeof(x)); /* there was data to read */
      SerialError(Port, old);
    }
    else
      read(Port, &x, sizeof(x)); /* there was data to read */

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

uint8_t RcvPacket(int port, uint8_t *pkt, struct termios *old) {
   uint8_t  Ck1, Ck2, N, i;
  
   waitstart(port, old);                          // Wait for start
   N = GetByte(port, old);                        // Get the packet size
   Ck1 = 0;                              // Start with checksum at zero
   for (i=0; i<N-1; i++) {
      *pkt = GetByte(port, old);                  // Get next byte, save it in Packet buffer
      Ck1 += *pkt;                       // Accumulate running checksum
      pkt++;                             // next byte
   }
   Ck2 = GetByte(port, old);                      // Get the senders checksum                        
   Ck1 &= 0xff;
   Ck1 %= 256;
   if (Ck1 == Ck2) { return N; } else { return 0; }
}

void waitstart(int port, struct termios *old) {             // This waits for soh and then stx in that order
  uint8_t c1,c2=0;
  while ((c1!=_SOH) || (c2!=_STX)){
    c1 = c2; 
    c2 = GetByte(port, old);
  }
  return; 
}

int WaitAck(int port,uint8_t *pac, struct termios *old) {
    uint8_t PSize;

    PSize = RcvPacket(port, pac, old);                   
    if (PSize == 0) {
       printf("*** Zero length packet\n> ");
       return False;
    }                
    if (pac[2]==_NACK) {
       printf("*** Received a nack from the C3\n> ");
       return False;
    }
    return True;
}

void PrintPkt(unsigned char *pkt){
  int         i,plen;
  plen = *pkt;
  printf("packet length %i  ",plen);
  for(i=0; i<plen; i++) printf("<%02x>", *pkt++);
  printf("\n");
  return;
}

int make_schedule_frame(uint8_t *pkt,uint8_t *data,int len,int day,int channel,uint32_t *sch){
    int     i,ii;
    _packed N;

    data[0] = _SCHEDULE_F;
    data[1] = (uint8_t)day;
    data[2] = (uint8_t)channel;
    data[3] = (uint8_t)*sch;
    ii = 4;
    for(i = data[3]+1; i > 0; i--){
      N.MyLong = *sch++;
      PackLong(&data[ii], N);
      ii += 4;
    }
    BuildPkt(ii, data,pkt);    
//    packet_make(pkt,data,ii);
    return 0;
  }    

/********************************************************
 **              Pack / UnPack  Routines               ** 
 **  Insert / remove propeller long from the Packet    **
 ********************************************************/
void PackLong(uint8_t *p, _packed N) { // N - 4 byte long,   p - insertion point
    *p = N.MyByte[0];  p++;
    *p = N.MyByte[1];  p++;
    *p = N.MyByte[2];  p++;
    *p = N.MyByte[3];  p++;
}

int UnPackLong(uint8_t *p) {   // p pointer to start of 4 byte long
    _packed N; 
    N.MyByte[0] = *p;  p++;
    N.MyByte[1] = *p;  p++;
    N.MyByte[2] = *p;  p++;
    N.MyByte[3] = *p;  p++;
    return N.MyLong;
}
