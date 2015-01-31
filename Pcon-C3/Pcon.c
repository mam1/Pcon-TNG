/*
 * Pcon.c
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */


#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include "propeller.h"
#include "Pcon.h"
// #include "test.h"
#include "simpletools.h"                      // Include simple tools
#include "simpletext.h"
#include "fdserial.h"

#define RX                      1
#define TX                      0
#define MODE                    0
#define BAUD                 9600
#define SERIAL_BUFFER_SIZE    128
#define START_COMMAND         27

// /*************************** drivers  ********************************/
// extern _Driver _FullDuplexSerialDriver;
// extern _Driver _FileDriver;
// _Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL}; 

/***************************** externals ******************************/

int main(int argc, char *argv[]){
    fdserial             *C3port;
    int             C3byte;
    int             out_byte;
    int             *size;
    uint8_t          s[4];
    int              sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_MAX_SCHEDULE_RECS+1];

    int             i;
    int             c;

    sleep(1);
    printf("\033\143"); //clear the terminal screen, preserve the scroll back
    disp_sys();

    printf("schedule size %i bytes\r\n",sizeof(sch));
    size = &s[0];
    printf("open serial port\n");
    C3port = fdserial_open(RX, TX, MODE, BAUD); //open io port to C3
    printf("C3port = %i\n",C3port);
    fdserial_rxFlush(C3port);           // flush input buffer
    fdserial_txFlush(C3port);           // flush input buffer
    // out_byte = PING;
    // printf("out byte <%u>\n",out_byte);
    // fdserial_txChar(C3port, out_byte);
    // printf("byte sent \n");
    while(1){
        printf("wait for anything from the bone\n");
        while (fdserial_rxReady(C3port) == 0);      //wait for something to show up in the buffer
        C3byte = fdserial_rxChar(C3port);           //grab a byte
        printf("got a <%u> from the bone\n",C3byte);
        switch(C3byte){
            case PING:
                out_byte = ACK;
                printf("sending ACK <%u>\n",out_byte);
                fdserial_txChar(C3port, out_byte); 
                break;
            case WRITE_SCH:
                for(i=0;i<4;i++){
                    while (fdserial_rxReady(C3port) == 0);      //wait for something to show up in the buffer
                    s[i] = fdserial_rxChar(C3port);           //grab a byte 
                    printf("read <%u> from the bone\r\n",s[i]);
                }
                printf("*** received a size of %i\n",*size);
                for(i=0;i<*size;i++){
                    c = fdserial_rxChar(C3port); 
                    printf("%i\r\n",(int)c);      
                }
                break;

            case PUSH_STATS:
            default:
                printf("unknown command received from the bone <%u>\n",C3byte);
        }
    }

    printf("\nnormal termination\n");
    return 0;
}

void disp_sys(void) {
	printf("\n*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

// void term(FdSerial_t *fd){
//     s_close(fd);
//     printf("serial port closed \n");
//     return;
// }


