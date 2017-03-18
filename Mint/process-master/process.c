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
#include "Pcon.h"
#include "typedefs.h"


int main (void) {

	FILE 			*sensor_data;
 	_SEN_DAT_REC 	buffer;

	int 			rcnt;
	// int 			snum;
	float 			avg_t[_MAX_SENSOR_ID];
	float 			sum_t[_MAX_SENSOR_ID];
	float 			max_t[_MAX_SENSOR_ID];
	float 			min_t[_MAX_SENSOR_ID];
	time_t 			c_date[_MAX_SENSOR_ID];
	time_t 			f_date[_MAX_SENSOR_ID];
	//  int 			sfound[_NUMBER_OF_SENSORS];
	int 			i;
	int 			hit_cnt[_MAX_SENSOR_ID];
	struct tm		tmf, tmc;

	printf("\r\n\nprocess_master v 0.0.5\r\n\n");
	printf("    opening %s\n\r",_SENSOR_MASTER_FILE_NAME);

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
	}
	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1)
	{
		rcnt++;
		if(buffer.sensor_id < 0 || buffer.sensor_id > _MAX_SENSOR_ID)
		{
			printf("  record %i sensor id out of range  <%i>\n", rcnt, buffer.sensor_id);
			if(fread(&buffer, sizeof(buffer), 1, sensor_data) != 1){
				printf("*** input file screwed up\r\n");
				return 1;
			}
				 
		}
		hit_cnt[buffer.sensor_id] += 1;
		sum_t[buffer.sensor_id] += buffer.temp;

		if(buffer.temp > max_t[buffer.sensor_id])
			max_t[buffer.sensor_id] = buffer.temp;

		if(buffer.temp < min_t[buffer.sensor_id])
			min_t[buffer.sensor_id] = buffer.temp;


		if(buffer.ts > f_date[buffer.sensor_id]){
			f_date[buffer.sensor_id] = buffer.ts;
		}

		if(buffer.ts > c_date[buffer.sensor_id]){
			c_date[buffer.sensor_id] = buffer.ts;
		}
	}
	printf("    %i records read\n\r", rcnt);
	fclose(sensor_data);
	printf("    closing %s\n\r",_SENSOR_MASTER_FILE_NAME);

	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0)
			avg_t[i] = sum_t[i] / hit_cnt[i];

	printf("\r\n   summery sensor values\n\r");
	printf("\n%6s%6s%7s%7s         %17s        %22s\n\r","ID","ave","max","min","first read","last read");
	printf("    --------------------------------------------------------------------------\n\r");
	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0){
			tmf = *localtime(&f_date[i]);
			tmc = *localtime(&c_date[i]);
			printf("%6i %6.1f %6.1f  %5.1f %4d:%02d:%02d-%02d/%02d/%02d %4d:%02d:%02d-%02d/%02d/%02d\n\r", 
				i, avg_t[i], max_t[i], min_t[i], tmf.tm_hour, tmf.tm_min, tmf.tm_sec, tmf.tm_mon, tmf.tm_mday, tmf.tm_year,
				tmc.tm_hour, tmc.tm_min, tmc.tm_sec, tmc.tm_mon, tmc.tm_mday, tmc.tm_year);
		}

	printf("\r\n %i records processed \r\n normal termination",rcnt);

  	return (0);
}

