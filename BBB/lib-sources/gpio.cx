#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "bitlit.h"
#include "gpio.h"

// /****************************************************************
//  * gpio_export
//  ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

// /****************************************************************
//  * gpio_unexport
//  ****************************************************************/
// int gpio_unexport(unsigned int gpio)
// {
// 	int fd, len;
// 	char buf[MAX_BUF];
 
// 	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
// 	if (fd < 0) {
// 		perror("gpio/export");
// 		return fd;
// 	}
 
// 	len = snprintf(buf, sizeof(buf), "%d", gpio);
// 	write(fd, buf, len);
// 	close(fd);
// 	return 0;
// }

// /****************************************************************
//  * gpio_set_dir
//  ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];
 
	// len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
	snprintf(buf,sizeof(buf),"/sys/class/gpio/gpio%d/direction",gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}