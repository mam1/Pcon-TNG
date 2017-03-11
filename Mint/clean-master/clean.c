
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

#define 	d 		4

_SEN_DAT_REC	s[d];
int 			s_index = {0};

int pushit(_SEN_DAT_REC *ss, int i){

	s[i] = *ss;
	if(i+1 > d)
		i = 0;
	else
		i += 1;;

	return i;
}



int main (void) {

	FILE 			*sensor_data;
	_SEN_DAT_REC 	buffer;
	int 			rcnt;
	int 			i;

	printf("\r\n\nclean_master v 0.0.6\r\n\n");
	printf("  opening %s\n\r",_SENSOR_MASTER_FILE_NAME);

	sensor_data = fopen(_SENSOR_MASTER_FILE_NAME,"r");
	if(sensor_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("*** attempting to open %s\n\n application terminated\n\n", _SENSOR_MASTER_FILE_NAME);
		return 1;
	}

	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1){
		rcnt += 1;
		s_index = pushit(&buffer,s_index);
		if(rcnt > d){
			for(i=0;i<d;i++)
				printf("<%i>  %02i:%02i:%02i  %s %02i/%02i/%02i sensor %i temp %0.2f humidity %0.2f\n",i,
	       				buffer.ts.tm_hour, buffer.ts.tm_min, buffer.ts.tm_sec, day_names_long[buffer.ts.tm_wday], 
	       				buffer.ts.tm_mon, buffer.ts.tm_mday, buffer.ts.tm_year, buffer.sensor_id, buffer.temp, buffer.humidity);
			if(rcnt > 25){
				fclose(sensor_data);
				return 0;
			}
			
		}
	}

	fclose(sensor_data);
	printf("  %i records read\n  normal termination\n", rcnt );

  return 0;
}

	// 	printf("  %02i:%02i:%02i  %s %02i/%02i/%02i sensor %i temp %0.2f humidity %0.2f\n",
	//        buffer.ts.tm_hour, buffer.ts.tm_min, buffer.ts.tm_sec, day_names_long[buffer.ts.tm_wday], 
	//        buffer.ts.tm_mon, buffer.ts.tm_mday, buffer.ts.tm_year, buffer.sensor_id, buffer.temp, buffer.humidity);
	// }