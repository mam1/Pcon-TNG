
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

/* globals */
_TERMIOS                  oldtio;
_TERMIOS                  newtio;
int                       Serial;
  
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

// /* initialize UART */  
int SerialInit(char *device, int bps, _TERMIOS *old) { 

    int               ret;
    // int               v24;

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

    // /* Set ready to read/write */
    // v24 = TIOCM_DTR | TIOCM_RTS;
    // ret = ioctl(Serial, TIOCMBIS, &v24);
    // if (ret < 0) {
    //   perror("ioctl(TIOCMBIS)");
    //   SerialError(Serial,&oldtio);
    // }

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


int main(void){

  int          c, cc;

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
        cc = _SOH;
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
        // printf("sending ping packe frame to UART\n");
        PutByte(Serial,_SOH);
        PutByte(Serial,_STX);
        cc = 3;
        // printf("sending <%02x> to UART1\n> ",cc);
        PutByte(Serial,cc);
        PutByte(Serial,_PING_F);
        PutByte(Serial,_PING);
        printf("ping frame sent to UART1\n> ");
        break;
      case 'r':
        printf("receive char from UART1\n> ");
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