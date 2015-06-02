
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>		//sleep
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
// #include <sys/ioctl.h>
#include "../shared/shared.h"


/* typedefs */
typedef struct termios _TERMIOS;

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

typedef struct {
  uint8_t         f_type;
  uint8_t         ping;
} _ping_frame;

/* globals */
_TERMIOS                  oldtio;
_TERMIOS                  newtio;
int                       Serial;
uint8_t                   pkt[_MAX_PACKET];
uint8_t                   RcvPkt[_MAX_PACKET];
 _ping_frame              ping_frame = {.f_type = _PING_F, .ping = _ACK};

  
/*************************  serial io routines *********************************/
void SerialError (int S, _TERMIOS *old){
    system("/bin/stty cooked");     	//switch to buffered input
    system("/bin/stty echo");     		//turn on terminal echo
    if(S != -1){						          //if we have a valid handle
      tcsetattr(S, TCSANOW, old);     //restore old tremios settings
      close(S);
    }
    printf("*** application terminated\r\n\n");
    exit(-1);
  }

/* initialize UART */  
int SerialInit(char *device, int bps, _TERMIOS *old) {
    int               ret;
	/* Load the pin configuration */
    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      Serial = -1;
      SerialError(-1,&oldtio);
    }
	/* open UART1 */
        // fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY  | O_NDELAY);
    Serial = open(device, O_RDWR | O_NOCTTY); //Open device in read/write mode
    if (Serial == -1) {
      printf("Error - Unable to open '%s'.\n", device);
      exit(1); 
    }
    ret = tcgetattr(Serial, old);            //save old tremios settings
    if (ret < 0) {
      printf("*** problem saving old tremios settings\r\n");
      SerialError(Serial,&oldtio);
    }
    bzero(&newtio, sizeof(newtio)); // clear struct for new port settings 
    tcgetattr(Serial, &newtio);
    newtio.c_cflag = bps | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(Serial, TCIFLUSH);
    tcsetattr(Serial, TCSANOW, &newtio);

    return Serial;     
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




int main(void){

  int          c, cc;

  uint8_t       *p;

  p = pkt;
  // *p++ = _SOH;
  // *p++ = _STX;
  *p++ = 2;
  *p++ = _PING_F;
  *p++ = _PING;

  printf("\n*****************************************************************************\n\n");
	printf("start test of sending a serial stream to bone UART1 <%s>\ninitializing UART1\n", _MODEMDEVICE);
	Serial = SerialInit(_MODEMDEVICE,B9600,&oldtio);
	printf("UART1 initialized\n");

  // /* set up unbuffered io */
  // fflush(stdout);
  // system("stty -echo");         //turn off terminal echo
  // system("/bin/stty raw");        // use system call to make terminal send all keystrokes directly to stdin
  // int flags = fcntl(STDOUT_FILENO, F_GETFL);
  // fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);


  printf("\n  s - send _SOH to UART1\n");
  printf("  h - send packet header to UART1\n");
  printf("  p - send ping packet to UART1\n");
  printf("  r - recieve a byte from UART1\n");
  printf("  q - terminate program\n\n> ");
  for(;;){
    while((c = getchar()) == '\n');                   //eat newlines
    switch(c){
      case 'q':
        /* cleanup and teriminate */
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
        if(WaitAck(Serial))
            printf("recieved ack from the prop\n");
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