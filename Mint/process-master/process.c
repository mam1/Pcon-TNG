
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "shared.h"
// #include "ipc.h"
#include "Pcon.h"
// #include "bitlit.h"
// #include "PCF8563.h"
// #include "gpio.h"
// #include "led.h"
// #include "schedule.h"

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

	int 			rcnt;
	int 			snum;
	int 			avg[_NUMBER_OF_SENSORS];
	int 			sum[_NUMBER_OF_SENSORS];
	int 			sfound[_NUMBER_OF_SENSORS];
	int 			i;



	printf("\nprocess sensor data v 0.0.3\r\n");
	printf("opening %s\n",_SENSOR_MASTER_FILE_NAME);

	sensor_data = fopen(_SENSOR_MASTER_FILE_NAME,"r");
	if(sensor_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _SENSOR_MASTER_FILE_NAME);
		return 1;
	}
	rcnt = 0;

	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1){
		rcnt++;
		if(buffer.sensor_id < 0 || buffer.sensor_id > (_NUMBER_OF_SENSORS - 1))
			printf("  sensor id out of range  <%i>\n", buffer.sensor_id);
		else
			switch(buffer.sensor_id){
			case 0:
				fwrite()
				if(fwrite(&buffer, sizeof(buffer), 1, cgi_data) != 1)
					printf("*** error writing to %s\n", sensor_log_file); 
				else 
					printf(" CGI: %i:%i:%i  %i/%i/%i\n\r", 
						ipc_ptr->s_dat[(int)l_num].ts.tm_hour, 
						ipc_ptr->s_dat[(int)l_num].ts.tm_min, 
						ipc_ptr->s_dat[(int)l_num].ts.tm_sec, 
						ipc_ptr->s_dat[(int)l_num].ts.tm_mon, 
						ipc_ptr->s_dat[(int)l_num].ts.tm_mday, 
						ipc_ptr->s_dat[(int)l_num].ts.tm_year);
					printf(" CGI: data logged to %s\n\r", sensor_log_file);

				fclose(cgi_data);
		}
			sfound[buffer.sensor_id] = 1;

		sum[buffer.sensor_id] += buffer.temp;
		// printf("  %02i:%02i:%02i  %s %02i/%02i/%02i sensor %i temp %i humidity %i\n",
	 //       buffer.ts.tm_hour, buffer.ts.tm_min, buffer.ts.tm_sec, day_names_long[buffer.ts.tm_wday], 
	 //       buffer.ts.tm_mon, buffer.ts.tm_mday, buffer.ts.tm_year, buffer.sensor_id, buffer.temp, buffer.humidity);
	}

	fclose(sensor_data);

	for(i=0; i<_NUMBER_OF_SENSORS; i++)
		avg[i] = sum[i] / rcnt;

	printf("closing %s\n",_SENSOR_MASTER_FILE_NAME);
	printf("%i records processed\n\r", rcnt);
	printf("    average sensor value\n");
	for(i=0; i<_NUMBER_OF_SENSORS; i++)
		printf("      sensor %i <%i>\n", i, avg[i]);

  return (0);
}

