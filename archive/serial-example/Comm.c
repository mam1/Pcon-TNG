/******************************************************************************************


                ##### RIVAL - RASPBERRY PI and PROPELLER ROBOT #####
                            Ray Tracy Feb 2015

   This is Rival.c the CGI command processor, it runs on the Raspberry Pi processor.
   Rival.spin is the firmware, it runs on the Parallax Propeller microprocessor.

     *** This version of the bot does not have an arm.

   The intent of this software is to kinda mimic the operation of the Mars rovers.
   Operationally a set of commands are sent to the rover which then carrys out that set
   of commands communicates the results and then waits for further instructions.

   HARDWARE:
     The P8X32A propeller microprocessor provides:
     interfaces to the LCD display, a WS2812 based LED display, three pings, motors and wheels 
     equiped with QME-01 encoders.

     The RASPBERRY PI is equiped with a wireless network interface, the camera and the
     pan/tilt servo controller and interface to the propeller P8X32A.


   FIRMWARE:
     The firmware (on the P8X32A) provides two way packet based communication between the
     Propeller and the RPI, and the low level interfaces to all hardware, Move and Turn
     routines utilizing a PID drive subsystem are part of the firmware as well.
     On the RPI side: There is a packet based communications subsystem which provides two 
     way communications with the P8X32A, an web based html control panel and CGI interface
     routines to each of the subsystems is provided. 

                             WS2812 COLOR TABLE
                COLOR           VALUE     INDEX         WHEEL POSITION
                red           $FF_00_00     1          000 degrees north
                orange        $FF_7F_00     2          030
                yellow        $FF_FF_00     3          060
                GreenYellow   $7F_FF_00     4          090 degrees east
                green         $00_FF_00     5          120
                greencyan     $00_FF_7F     6          150
                cyan          $00_FF_FF     7          180 degrees south
                bluecyan      $00_7F_FF     8          210
                blue          $00_00_FF     9          240
                bluemagenta   $7F_00_FF     10         270 degrees west
                magenta       $FF_00_FF     11         300
                redmagenta    $FF_00_7F     12         330


**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>

char version[]="---Rival.c  V1.65---";

// ======<< Ascii control Characters >>=====
#define      soh  0x01
#define      stx  0x02
#define      etx  0x03
#define      eot  0x04
#define      ack  0x06
#define      nak  0x15
#define      syn  0x16

#define      False 0
#define      True 1
#define      baudrate   B9600               // B57600 B115200  B230400 
#define      usbPort    "/dev/ttyUSB"
#define      uartPort   "/dev/ttyO1"       // For use with GPIO pins

typedef unsigned char Byte;
typedef enum ledclr leds;
typedef union { unsigned int MyLong; unsigned char MyByte[4]; } packed;
int           Port ;
Byte SndPkt[100];
Byte RcvPkt[100];

// ***** POST DATA VARIABLES *****
char String[25];
int  Command, Number;

/************************************
 **       UTILITY ROUTINES         **
 ************************************/
void BuildPkt(Byte N, unsigned char *pkt) {
   Byte i;
   for (i=0; i<N; i++) { *pkt = (i + 0x40); pkt++; }
}

void ShoPkt(Byte N, unsigned char *pkt ) {
   int i,ck;
   printf("SndPkt: [%x]",N);
   for (i=0; i<N; i++) { printf(" %x",*pkt); ck+=*pkt; pkt++; }   // Packet data
   printf(" {%x}\n",(ck%256));
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
   PutByte( N );                        // Send packet size
   Ck1 = 0;                             // Clear the checksum
   for (i=0; i<N; i++) {
       PutByte( *pkt );                 // Send the next data character 
       Ck1 += *pkt;                     // Accumulate the checksum
       pkt++;                           // next byte
   }
   Ck1 %= 256;
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
    struct termios options;
    int Serial;

    Serial = open(device, O_RDWR | O_NOCTTY ); //Open device in read/write mode
    if (Serial == -1) { printf("Error - Unable to open '%s'.\n", device); exit(1); }
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

/*********************
 **      MAIN       **
 *********************/
void main( int argc, char *argv[] ) {

    printf("%i arguments\n",argc);

    Port = SerialInit(uartPort, baudrate);
    GetPostData();
}
