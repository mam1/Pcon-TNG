#ifndef IPC_H_
#define IPC_H_

#define SYSFS_GPIO_DIR "/sys/class/gpio"

// #define POLL_TIMEOUT (3 * 1000) /* 3 seconds */

#define MAX_BUF 100

#define _LED_0		66	//map gpio to led 
#define _LED_1		69	//map gpio to led	
#define _LED_2		68	//map gpio to led		
#define _LED_3		67	//map gpio to led	


int gpio_export(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);

#endif