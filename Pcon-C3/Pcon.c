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
#include "../Pcon-share/schedule.h"
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

/***************** global code to text conversion ********************/
const char *day_names_long[7] = {
     "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
const char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
const char *onoff[2] = {"off"," on"};
const char *con_mode[3] = {"manual","  time","time & sensor"};
const char *sch_mode[2] = {"day","week"};
const char *c_mode[4] = {"manual","  time","   t&s"," cycle"};

// /*************************** drivers  ********************************/
// extern _Driver _FullDuplexSerialDriver;
// extern _Driver _FileDriver;
// _Driver *_driverlist[] = {&_FullDuplexSerialDriver,&_FileDriver,NULL}; 

/***************************** externals ******************************/

int main(int argc, char *argv[]){
    fdserial            *C3port;
    int                 C3byte;
    int                 out_byte;
    int                 *size;
    uint8_t             s[4], *load_byte;
    uint32_t            sch[_DAYS_PER_WEEK][_NUMBER_OF_CHANNELS][_SCHEDULE_SIZE], *sch_ptr;
    SYS_DAT             sys_dat;

    int             i, get_bytes;
    int             c;
    uint8_t             *buf_ptr;
    int                 int_buf;

    sleep(1);
    printf("\033\143"); //clear the terminal screen, preserve the scroll back
    disp_sys();

    sch_ptr = (uint32_t *)sch;

    printf("schedule size %i bytes\r\n",sizeof(sch));
    size = (int *)&s[0];
    printf("open serial port\n");
    C3port = fdserial_open(RX, TX, MODE, BAUD); //open io port to C3
    printf("C3port = %i\n",C3port);
    fdserial_rxFlush(C3port);           // flush input buffer
    fdserial_txFlush(C3port);           // flush transmit buffer

    while(1){
        printf("wait for anything from the bone\n");
        while (fdserial_rxReady(C3port) == 0);      //wait for something to show up in the buffer
        C3byte = fdserial_rxChar(C3port);           //grab a byte
        printf("got a <%u> from the bone\n",C3byte);
        switch(C3byte){
            case _PING:
                printf("received a _PING, sending ACK <%u>\n\n",out_byte);
                out_byte = _ACK;
                fdserial_txChar(C3port, out_byte); 
                break;
            case _RECEIVE_SYS_DAT:
                get_bytes = sizeof(sys_dat);
                load_byte = (uint8_t *)&sys_dat;
                printf("received a _RECIEVE_SYS_DAT, reading %i bytes from the bone\r\n",get_bytes);
                out_byte = _ACK;
                fdserial_txChar(C3port, out_byte);

                while(get_bytes-- > 0){
                    out_byte = _ACK;
                    fdserial_txChar(C3port, out_byte);
                    while (fdserial_rxReady(C3port) == 0);                            
                    *load_byte = fdserial_rxChar(C3port);         //grab a byte
                    load_byte++;                    
                }
                printf("system data %i.%i.%i loaded\n",sys_dat.major_version, sys_dat.minor_version, sys_dat.minor_revision);
                break;
            case _WRITE_SCH:
                get_bytes = sizeof(sch);
                printf("received a _WRITE_SCH, reading %i bytes from the bone\r\n",get_bytes);

                out_byte = _ACK;
                fdserial_txChar(C3port, out_byte);

                while(get_bytes-- > 0){
                    out_byte = _ACK;
                    fdserial_txChar(C3port, out_byte);
                    while (fdserial_rxReady(C3port) == 0){
                        // pause(20);
                    };                            
                    *sch_ptr = fdserial_rxChar(C3port);         //grab a byte

                    // printf("<%u> recieved\r\n", *sch_ptr);
                    sch_ptr++;                    
                    // printf("\n\rbyte written\n\rget_bytes = %i\r\n",get_bytes);

                }
                printf("schedule received from the bone\n\r");
                // disp_all_schedules(CMD_FSM_CB *cb,uint32_t *sch)
                break;
            case _INT:
                get_bytes = 4;
                buf_ptr = (uint8_t *)&int_buf;
                printf("received a _INT, reading %i bytes from the bone\r\n",get_bytes);
                while(get_bytes-- > 0){
                    *buf_ptr++ = fdserial_rxTime(C3port, 200);  //grab a byte
                    if(*buf_ptr == -1){
                        printf("*** read time out in receiving int\n");
                        *buf_ptr = '\0';
                    }                           
                }
                printf("received %i\n",int_buf);
                break;
            case _SYSTEM:
                printf("received a request to reload local copy of system data from the bone\n");
                int_buf = get_int(C3port);
                printf("bone sending %i bytes\n",int_buf);
                get_block(C3port,&sys_dat, int_buf);
                printf("system data %i.%i.%i loaded\n",sys_dat.major_version, sys_dat.minor_version, sys_dat.minor_revision);

                break;
            case _PUSH_STATS:
                break;
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


/* get an int from the bone */
int get_int(fdserial *fd){
    int         get_bytes = 4;
    int         int_buf = 0;
    uint8_t     *buf_ptr;

    buf_ptr = (uint8_t *)&int_buf;
    // printf("receiving a _INT, reading %i bytes from the bone\r\n",get_bytes);
    while(get_bytes-- > 0){
        *buf_ptr = fdserial_rxTime(fd, 200);  //grab a byte
        if(*buf_ptr == -1){
            printf("*** read time out in get_int\n");
            *buf_ptr = '\0';
        }
        // printf("received <%u>",*buf_ptr);
        buf_ptr++;                           
    }
    // printf("received %i\n",int_buf);    
    return int_buf;
}

/* get a block of data from the bone */
void get_block(fdserial *fd,uint8_t *buf_ptr,int count){ 
   while(count-- > 0){ 
        while (fdserial_rxReady(fd) == 0);      //wait for something to show up in the buffer
        *buf_ptr++ = fdserial_rxChar(fd);       //grab a byte
    }
    return;
}

// void term(FdSerial_t *fd){
//     s_close(fd);
//     printf("serial port closed \n");
//     return;
// }


