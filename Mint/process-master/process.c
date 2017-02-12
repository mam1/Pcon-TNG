/*
process.c

	read master log and:
		calculate statistics on sensor values

*/


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


// /***************** global code to text conversion ********************/
// char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
// char *onoff[2] = {"off", " on"};
// char *con_mode[3] = {"manual", "  time", "time & sensor"};
// char *sch_mode[2] = {"day", "week"};
// char *mode[4] = {"manual", "  time", "   t&s", " cycle"};

/* return a pointer to the most current of 2 time data structures */
_tm *cdate(_tm *d1, _tm *d2){
	if(d1->tm_year > d2->tm_year)
		return d1;
	if(d1->tm_year < d2->tm_year)
		return d2;

	if(d1->tm_mon > d2->tm_mon)
		return d1;
	if(d1->tm_mon < d2->tm_mon)
		return d2;

	if(d1->tm_mday > d2->tm_mday)
		return d1;
	if(d1->tm_mday < d2->tm_mday)
		return d2;

	if(d1->tm_hour > d2->tm_hour)
		return d1;
	if(d1->tm_hour < d2->tm_hour)
		return d2;

	if(d1->tm_min > d2->tm_min)
		return d1;
	if(d1->tm_min < d2->tm_min)
		return d2;

	if(d1->tm_sec > d2->tm_sec)
		return d1;
	if(d1->tm_sec < d2->tm_sec)
		return d2;

	return NULL;
}

int main (void) {

	FILE 			*sensor_data;
 	_SEN_DAT_REC 	buffer;

	int 			rcnt;
	int 			snum;
	float 			avg_t[_MAX_SENSOR_ID];
	float 			sum_t[_MAX_SENSOR_ID];
	float 			max_t[_MAX_SENSOR_ID];
	float 			min_t[_MAX_SENSOR_ID];
	_tm 			c_date[_MAX_SENSOR_ID];


	int 			sfound[_NUMBER_OF_SENSORS];
	int 			i;
	int 			hit_cnt[_MAX_SENSOR_ID];


	printf("\nprocess sensor data v 0.0.3\r\n");
	printf("opening %s\n",_SENSOR_MASTER_FILE_NAME);

	sensor_data = fopen(_SENSOR_MASTER_FILE_NAME,"r");
	if(sensor_data == NULL){
		printf("  Error: %d (%s)\n", errno, strerror(errno));
		printf("    attempting to open %s\n\n application terminated\n\n", _SENSOR_MASTER_FILE_NAME);
		return 1;
	}
	rcnt = 0;
	for(i=0; i<_MAX_SENSOR_ID; i++){
		hit_cnt[i] = 0;
		avg_t[i] = 0;
		sum_t[i] = 0;
		max_t[i] = 0;
		min_t[i] = 999;
		c_date[i].tm_year = 2000;
	}

	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1){
		rcnt++;
		printf("    %06i records read\r", rcnt);
		if(buffer.sensor_id < 0 || buffer.sensor_id > _MAX_SENSOR_ID)
		{
			printf("  sensor id out of range  <%i>\n", buffer.sensor_id);
			return 1;
		}

		hit_cnt[buffer.sensor_id] += 1;
		sum_t[buffer.sensor_id] += buffer.temp;

		if(buffer.temp > max_t[buffer.sensor_id])
			max_t[buffer.sensor_id] = buffer.temp;

		if(buffer.temp < min_t[buffer.sensor_id])
			min_t[buffer.sensor_id] = buffer.temp;

		c_date[buffer.sensor_id] = *cdate(&buffer.ts, &c_date[buffer.sensor_id]);

	}
	fclose(sensor_data);
	printf("closing %s\n",_SENSOR_MASTER_FILE_NAME);

	printf("\n%i records read\n\r", rcnt);

	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0)
			avg_t[i] = sum_t[i] / hit_cnt[i];


	printf("   sensor values\n");
	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0)
			printf("      sensor ID %i average <%0.2f>  max <%0.2f>  min <%0.2f> last update <%02i:%02i:%02i-%02i/%02i/%02i>\n", 
				i, avg_t[i], max_t[i], min_t[i], c_date[i].tm_hour, c_date[i].tm_min, c_date[i].tm_sec, c_date[i].tm_mon, c_date[i].tm_mday, c_date[i].tm_year);


	printf("%s\n", "\nnormal termination\n");
  	return (0);
}


		// else
		// 	switch(buffer.sensor_id){
		// 	case 0:
		// 		fwrite()
		// 		if(fwrite(&buffer, sizeof(buffer), 1, cgi_data) != 1)
		// 			printf("*** error writing to %s\n", sensor_log_file); 
		// 		else 
		// 			printf(" CGI: %i:%i:%i  %i/%i/%i\n\r", 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_hour, 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_min, 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_sec, 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_mon, 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_mday, 
		// 				ipc_ptr->s_dat[(int)l_num].ts.tm_year);
		// 			printf(" CGI: data logged to %s\n\r", sensor_log_file);

		// 		fclose(cgi_data);
		// }
		// 	sfound[buffer.sensor_id] = 1;

		// sum[buffer.sensor_id] += buffer.temp;