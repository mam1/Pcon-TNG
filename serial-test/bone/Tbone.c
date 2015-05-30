



#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>		//sleep
#include <string.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include "../shared/shared.h"

/* globals */
struct tremios 		options, oldtio;
int 				Serial;

void SerialError (void){
    system("/bin/stty cooked");     		//switch to buffered input
    system("/bin/stty echo");     			//turn on terminal echo
    if(Serial != -1){						//if we have a valid handle
      tcsetattr(Serial, TCSANOW, &oldtio);  //restore old tremios settings
      close(Serial);
  }
    printf("*** application terminated\r\n\n");
    exit(-1);
  }

/* initialize UART */  
int SerialInit(char *device, int bps) { 

    int               ret;
	/* Load the pin configuration */
    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      Serial = -1;
      SerialError();
    }
	/* open UART1 */
    Serial = open(device, O_RDWR | O_NOCTTY ); //Open device in read/write mode
    if (Serial == -1) {
      printf("Error - Unable to open '%s'.\n", device);
      exit(1); 
    }
    ret = tcgetattr(Serial, &oldtio); //save old tremios settings
    if (ret < 0) {
      printf("*** problem saving old tremios settings\r\n");
      SerialError();
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

int main(void){

	printf("\n\nstart test of sending a serial stream to bone UART1 \ninitializing UART1\n");
	SerialInit("/dev/tty01",9600);
	printf("UART1 initialized");


	return 0;
}