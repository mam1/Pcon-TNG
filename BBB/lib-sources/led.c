

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>    //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include "bitlit.h"
#include "PCF8563.h"
#include "gpio.h"
#include "led.h"

#include "led.h"
#include "shared.h"

void led_write(int gpio, int state){
    char                    filename[100];
    int                     gpio_fd;
    uint8_t                 sbuf[2]={1,0};

    gpio_export(gpio);
    gpio_set_dir(gpio, 1);
    snprintf(filename,sizeof(filename),"/sys/class/gpio/gpio%d/value",gpio);
    gpio_fd = open(filename,O_WRONLY);
    printf("write reutrned %i\n",write(gpio_fd,sbuf,2));
    close(gpio_fd);
    return;
}

void led_set(int fd, int state){
    uint8_t               sbuf[2];

    printf("led_set called with fd = %i and state = %i\n",fd,state );
    if(state)
        sbuf[0] = 1;
    else
        sbuf[0] = 0;
    if(write(fd,sbuf,2) == -1)
        handle_error("led set");
    return;
}

int led_open(int gpio){
    char                    filename[100];
    int                     gpio_fd;

    gpio_export(gpio);
    gpio_set_dir(gpio, 1);
    snprintf(filename,sizeof(filename),"/sys/class/gpio/gpio%d/value",gpio);    // build the file name
    gpio_fd = open(filename,O_WRONLY);
    if(gpio_fd == -1)
        handle_error("open");

    return gpio_fd;
}