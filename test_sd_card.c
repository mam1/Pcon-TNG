/* 
* @Author: mam1
* @Date:   2014-12-31 14:15:27
* @Last Modified by:   mam1
* @Last Modified time: 2015-01-02 20:45:02
*/

#include <stdio.h>
#include "Pcon.h"
#include "typedefs.h"
#include "sd_card.h"
#include <string.h>

int main() {

	FILE			*fd;
	int             w_channel;                      //working channel number
	uint8_t         c_state[_NUMBER_OF_CHANNELS];   //chanel state arry
	SYS_DAT 		sys_dat;						//channel data and system version info

	printf("\n\nSD Card test\n");


 	// load_channel_data(_SYSTEM_DATA_FILE,&sys_dat);
 	printf("n = %c\n",sys_dat.c_data[2].name[2]);
 	sys_dat.c_data[2].name[2] = "z";
  	printf("n = %c\n",sys_dat.c_data[2].name[2]);	




    return 0;
}