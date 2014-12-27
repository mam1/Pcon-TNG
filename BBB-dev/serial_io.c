/*
 * serial_io.c
 *
 *  Created on: Nov 12, 2014
 *      Author: mam1
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>    //uint_8, uint_16, uint_32, etc.
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "Pcon.h"
#include "serial_io.h"

static struct termios oldtio, newtio;

int s_open(void){
    int fd;
    int ret;
    int   v24;

  /* Load the pin configuration */

    ret  = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    if(ret < 0){
      perror("echo uart1 > /sys/devices/bone_capemgr.9/slots");
      s_error(1);
    }

  /* Open modem device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.          */
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY  | O_NDELAY);
    if (fd < 0) {
    	perror(MODEMDEVICE);
    	s_error(2);
    }

  /* save old settings */
    ret = tcgetattr(fd, &oldtio);
    if (ret < 0) {
      perror("tcgetattr(oldtio)");
      s_close(fd);
      s_error(3);
    }

    bzero(&newtio, sizeof(newtio)); // clear struct for new port settings 

    cfsetispeed(&newtio, BAUDRATE);
    cfsetospeed(&newtio, BAUDRATE);

    newtio.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
    newtio.c_cflag |=  (CREAD | CLOCAL | CS8);
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_iflag |=  (INPCK | ISTRIP);
    newtio.c_iflag &= ~(ISTRIP | IXON | IXOFF | IGNBRK | INLCR | ICRNL | IGNCR);
    newtio.c_oflag &= ~(OPOST | ONLCR);

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

  // /* Configure serial interface */
  //   ret = tcgetattr(fd, &newtio);
  //   if (ret < 0) {
  //     perror("tcgetattr(newtio)");
  //     s_close(fd);
  //     exit(-1);
  //   }

  //  Configure serial interface wit old settings 
  //   ret = tcgetattr(fd, &newtio);
  //   if (ret < 0) {
  //     perror("tcgetattr(newtio)");
  //     s_close(fd);
  //     exit(-1);
  //   }

    // tcflush(fd, TCIFLUSH);
    ret = tcsetattr(fd,TCSANOW,&newtio);
    if (ret < 0) {
      perror("tcsetattr(newtio)");
      s_close(fd);
      s_error(4);
    }
    

  /* Set ready to read/write */
    v24 = TIOCM_DTR | TIOCM_RTS;
    ret = ioctl(fd, TIOCMBIS, &v24);
    if (ret < 0) {
      perror("ioctl(TIOCMBIS)");
      s_error(5);
    } 

    return fd;
}

void s_close(int fd){
    tcsetattr(fd, TCSANOW, &oldtio);
    close(fd);
    return;
}

void s_rbyte(int fd, uint8_t *byte){
  int    ret;
  int     i;

  printf("before read fd = %d, byte = <%u>\r\n",fd,*byte);

  // ret = read(fd,byte,1);
  // printf("\r\n  return <%d> byte <%u>\r\n",ret, *byte);
  // return;
  i =0;
  while(1){
    ret = read(fd,byte,1);
    if((ret < 0) && (i++ == READ_TRYS)){
      perror("\n*** serial read error ");
      s_error(fd);
    }
    if(ret > 0)
      return;
    usleep(10);
  }
  perror("\n*** serial read error no records read ");
  s_error(fd);
  return;
}

void s_wbyte(int fd, uint8_t *byte){
  int    ret;

  ret = write(fd,byte,1);
  if(ret < 0){
    perror("\n*** serial write error ");
    s_error(fd);
  }

  return;
}

void s_error(int fd){
  s_close(fd);
  term(3);
  return;
}

