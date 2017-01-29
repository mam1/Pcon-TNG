
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
// #include "shared.h"
// // #include "ipc.h"
// #include "Pcon.h"
// // #include "bitlit.h"
// #include "PCF8563.h"
// // #include "gpio.h"
// // #include "led.h"
// // #include "schedule.h"

#include "typedefs.h"


/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *mode[4] = {"manual", "  time", "   t&s", " cycle"};

int main (void) {

	FILE 			*sensor_data;
	struct{
		int 		sensor_id;
		int			temp;
		int			humidity;
		_tm 		ts;
	} buffer;

	sensor_data = fopen(_SENSOR_MASTER_FILE_NAME,"r");
	if(sensor_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _SENSOR_MASTER_FILE_NAME);
		return 1;
	}
	printf("  %s opened\n",_SENSOR_MASTER_FILE_NAME);
	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1){
		printf("  %02i:%02i:%02i  %s %02i/%02i/%02i sensor %i temp %i humidity %i\n",
	       buffer.ts.tm_hour, buffer.ts.tm_min, buffer.ts.tm_sec, day_names_long[buffer.ts.tm_wday], 
	       buffer.ts.tm_mon, buffer.ts.tm_mday, buffer.ts.tm_year, buffer.sensor_id, buffer.temp, buffer.humidity);
	}

	fclose(sensor_data);

  return (0);
}

