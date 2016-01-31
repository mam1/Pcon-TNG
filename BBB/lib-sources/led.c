#include "led.h"
#include "shared.h"

void led_set(int fd, int state){

    if(write(fd,&state,2) == 2)
        handle_error("write");
    return;
}

int led_open(int gpio){
    char                    filename[60];

    gpio_export(gpio);
    gpio_set_dir(gpio, 1);

    /* build the file name */
    snprintf(filename,sizeof(filename),"/sys/class/gpio/gpio%d/value",gpio);
    gpio_fd = open(filename,O_WRONLY);
    if(gpio_fd == -1)
        handle_error("open");

    return gpio_fd;
}