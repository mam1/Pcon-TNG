

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.

//char version[]="---Rival.c  V1.65---";

// ======<< Ascii control Characters >>=====
#define      soh  0x01
#define      stx  0x02
#define      etx  0x03
#define      eot  0x04
#define      ack  0x06
#define      nak  0x15
#define      syn  0x16

/* frame types */
#define _SCHEDULE_F     1 // replace a working schedule with schedule in frame, send ack to sender
#define _CHANNEL_F      2   // replace channel data with channel data in frame, send channel data back to sender
#define _TIME_F         3   // set real time clock to the time/date in frame, send ack to sender
#define _PING_F     4   // request for ping, send ping data in frame back to sender 
#define _ACK_F         5 //
#define _REBOOT_F      6 // reboot the C3, program load from EEPROM 

#define      False 0
#define      True 1
#define      baudrate   B9600               // B57600 B115200  B230400 
#define      usbPort    "/dev/ttyUSB"
#define      uartPort   "/dev/ttyO1"       // For use with GPIO pins

void SerialError (int,struct termios *);

typedef unsigned char Byte;
// typedef enum ledclr leds;
typedef union { unsigned int MyLong; unsigned char MyByte[4]; } packed;
int           Port ;
Byte SndPkt[254];
Byte RcvPkt[254];

// ***** POST DATA VARIABLES *****
// char String[25];
// int  Command, Number;

/************************************
 **       UTILITY ROUTINES         **
 ************************************/
void BuildPkt(Byte N, unsigned char *frame, unsigned char *pkt) {
   Byte i;
   *pkt++ = N + 1;
   for (i=0; i<N; i++) {
//     *pkt = (i + 0x40);
    *pkt++ = *frame++; 
   }
   return;
}

void ShoPkt(Byte N, unsigned char *pkt ) {
   int i,ck;
   // printf("SndPkt length: [%x]",N);
   for (i=0; i<N; i++) {
      printf(" %x",*pkt); 
      ck+=*pkt; 
      pkt++; 
   }   // Packet data
   printf(" {%x}\n",(ck%256));
   return;
}

void delay(ms) { usleep(ms*1000); return;}

/********************************************************
 **              Pack / UnPack  Routines               ** 
 **  Insert / remove propeller long from the Packet    **
 ********************************************************/
void PackLong( char* p, packed N ) { // N - 4 byte long,   p - insertion point
    *p = N.MyByte[0];  p++;
    *p = N.MyByte[1];  p++;
    *p = N.MyByte[2];  p++;
    *p = N.MyByte[3];  p++;
}

int UnPackLong( char* p ) {   // p pointer to start of 4 byte long
    packed N; 
    N.MyByte[0] = *p;  p++;
    N.MyByte[1] = *p;  p++;
    N.MyByte[2] = *p;  p++;
    N.MyByte[3] = *p;  p++;
    return N.MyLong;
}

/**************************************************************************************
 **                                                                                  **
 **                      Send / Receive Packet  Routines                             ** 
 **                   These routines Send or receive a Packet                        **
 **                                                                                  **
 ** -------------------------------------------------------------------------------- **
 **                      Packet Definition                                           **
 **     Each block begins with <soh> <stx> and a <count> followed by the data        **
 **     bytes and the <checksum>. There are no restrictions on the contents of       **
 **     The Packet. The checksum is the mod 256 sum of the data not including        **
 **     soh,stx,count or cksum, count does not include soh,stx,count or cksum.       **
 **     <packet>      ::= <packetstart><count><data><cksum>                          **
 **     <packetstart> ::= <headerstart><textstart>                                   **
 **     <headerstart> ::= soh                                                        **
 **     <textstart>   ::= stx                                                        **
 **     <count>       ::= <maxnumber>                                                **
 **     <cksum>       ::= Sum<data> mod 256                                          **
 **     <data>        ::= <byte> | <data><byte                                       **
 **     <byte>        ::= $00..$FF                                                   **
 **     <maxnumber>   ::= 0..255                                                     **
 **                                                                                  **
 **************************************************************************************/

Byte GetByte()  {char x;  read (Port, &x, sizeof(x) ); return x; }	
Byte PutByte(unsigned char c) { write(Port, &c, sizeof(c) ); }

/***************************************
 **       WAIT PACKET START           **
 ***************************************/
void waitstart() {             // This waits for soh and then stx in that order
   Byte c1,c2=0;
   while ((c1!=soh) || (c2!=stx)) { c1 = c2; c2 = GetByte(); }
}

/***************************************
 **          SEND PACKET              **
 ***************************************/
void SndPacket( Byte N, unsigned char *pkt ) {
   int  i;
   Byte Ck1;

   PutByte(soh);                        // Send start of packet
   PutByte(stx);                        // Send start of packet
   PutByte(N);                        // Send packet size
   // PutByte(_PING_F);
   Ck1 = 0;                             // Clear the checksum
   for (i=0; i<N; i++) {
       PutByte(*pkt);                 // Send the next data character 
       Ck1 += *pkt;                     // Accumulate the checksum
       pkt++;                           // next byte
   }
   Ck1 %= 256;
   *pkt = Ck1;
   PutByte( Ck1 );                      // Send the checksum
}

/************************************
 **       RECEIVE PACKET           **
 ************************************/
Byte RcvPacket(Byte *pkt) {
   Byte  Ck1, Ck2, N, i;
  
   waitstart();                          // Wait for start
   N = GetByte();                        // Get the packet size
   Ck1 = 0;                              // Start with checksum at zero
   for (i=0; i<N; i++) {
      *pkt = GetByte();                  // Get next byte, save it in Packet buffer
      Ck1 += *pkt;                       // Accumulate running checksum
      pkt++;                             // next byte
   }
   Ck2 = GetByte();                      // Get the senders checksum
   Ck1 %= 256;                           // Finish the checksum calculation
   if (Ck1 == Ck2) { return N; } else { return 0; }
}

/************************************
 **       Wait for ACK or NAK      **
 ************************************/
int WaitAck() {
    Byte PSize;
    PSize = RcvPacket(RcvPkt);                    // Receive Packet
    if ((PSize == 0) || (RcvPkt[0]==nak)) {
       printf("Cmd Failed\n");
       return False;
    }
    return True;
}

/***************************
 **    GET POST DATA      **
 ***************************/
// void GetPostData() {
//  char   postdata[300], *lenstr;
//  char   s[2] = "&";
//  char   *T1, *T2, *Tok00, *Tok01, *Tok02;
//  long   len;

// lenstr = getenv("CONTENT_LENGTH");
// printf("lenstr = %i\n",lenstr);
// if(lenstr == NULL) { printf("<P>***Error*** Cannot fetch POST string."); }
// else {
//    sscanf(lenstr, "%ld", &len);
//    fgets(postdata, len+1, stdin);
//    postdata[len+2] = 0;
// // POST data string --> Parameter1=Value&parameter2=value&...&parameterN=value
// //   printf("<p>POSTDATA:<br> %s</p>",postdata);
//    Tok00    = strtok( postdata, s );  //Command      Seperate Post parameters into individual tokens
//    Tok01    = strtok( NULL, s );   //Distance
//    Tok02    = strtok( NULL, s );   //String
// // Get Command 
//    T1    = strtok( Tok00, s);    // Name
//    T2    = strtok( NULL, s);    // value (still an ascii string)
//    Command = atoi( T2 );        // convert to integer and save
// // Get number
//    T1    = strtok( Tok01, s);
//    T2    = strtok( NULL, s);
//    Number = atoi( T2 );
// // Get String   
//    T1     = strtok( Tok02, s);    // Name
//    T2     = strtok( NULL, s);     // value - fixed length string
//    strncpy(String,T2,20);
//    }
// }

/************************************************************************************
                   ******************************
                   **    SERIAL PORT INIT      **
                   ******************************
        struct termios {
                tcflag_t c_iflag;               // input mode flags
                tcflag_t c_oflag;               // output mode flags
                tcflag_t c_cflag;               // control mode flags
                tcflag_t c_lflag;               // local mode flags
                cc_t c_line;                    // line discipline
                cc_t c_cc[NCCS];                // control characters
        };
************************************************************************************/
int SerialInit(char *device, int bps) { 
    struct termios    options, oldtio;
    int               Serial;
    int               ret,fd;



  /* Load the pin configuration */
    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      SerialError(-1, &oldtio);
    }

    Serial = open(device, O_RDWR | O_NOCTTY ); //Open device in read/write mode
    if (Serial == -1) {
      printf("Error - Unable to open '%s'.\n", device);
      exit(1); 
    }
    ret = tcgetattr(Serial, &oldtio); //save old tremios settings
    if (ret < 0) {
      printf("*** problem saving old tremios settings\r\n");
      // s_close(fd);
      SerialError(Serial, &oldtio);
    }
    tcgetattr(Serial, &options);
    options.c_cflag = bps | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 1;
    tcflush(Serial, TCIFLUSH);
    tcsetattr(Serial, TCSANOW, &options);
    return Serial;     
}

void SerialError (int fd, struct termios *oldtio){
    system("/bin/stty cooked");     //switch to buffered iput
    system("/bin/stty echo");     //turn on terminal echo
    if(fd != -1){
      tcsetattr(fd, TCSANOW, oldtio);  //restore old tremios settings
      close(fd);
  }
    printf("*** application terminated\r\n\n");
    exit(-1);
  }
/******************************************************************************/

int packet_print(uint8_t *pkt)
{
    int n;
    int len = *pkt++;
    printf("packet len %d : <", len);
    for (n = 0; n < len; n++) {
        printf(" %02x", *pkt++);
    }
    printf(">\n\r");
    return 0;
}


