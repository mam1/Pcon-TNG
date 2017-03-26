
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "bitlit.h"
#include "PCF8563.h"
// #include "gpio.h"
// #include "led.h"
// #include "schedule.h"
#include "BBBiolib.h"
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
	_SEN_DAT_REC	 buffer;
	struct tm 		tm;

	// int 		parm =2;

	sensor_data = fopen(_SENSOR_DATA_FILE_NAME,"rb");
	if(sensor_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _SENSOR_DATA_FILE_NAME);
		return 1;
	}
	printf("  %s opened\n",_SENSOR_DATA_FILE_NAME);
	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1){

		tm = *localtime(&buffer.ts);
		printf(" CGI: sensor %i, %i:%i:%i,  %i/%i/%i,  temp %0.2f,  humidity %0.2f\n\r",
			buffer.sensor_id, 
			tm.tm_hour, 
			tm.tm_min, 
			tm.tm_sec, 
			tm.tm_mon + 1, 
			tm.tm_mday, 
			tm.tm_year + 1900,
			buffer.temp,
			buffer.humidity);
		printf(" CGI: data logged to %s\n\r", _SENSOR_DATA_FILE_NAME);
		// printf("  %02i:%02i:%02i  %s %02i/%02i/%02i sensor %i temp %0.2f humidity %0.2f\n",
	 //       buffer.ts.tm_hour, buffer.ts.tm_min, buffer.ts.tm_sec, day_names_long[buffer.ts.tm_wday], 
	 //       buffer.ts.tm_mon, buffer.ts.tm_mday, buffer.ts.tm_year, buffer.sensor_id, buffer.temp, buffer.humidity);
	}

	fclose(sensor_data);

  return (0);
}

