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

/* return a pointer to the least current of 2 time data structures */
_tm *fdate(_tm *d1, _tm *d2){
	if(d1->tm_year < d2->tm_year)
		return d1;
	if(d1->tm_year > d2->tm_year)
		return d2;
	if(d1->tm_mon < d2->tm_mon)
		return d1;
	if(d1->tm_mon > d2->tm_mon)
		return d2;
	if(d1->tm_mday < d2->tm_mday)
		return d1;
	if(d1->tm_mday > d2->tm_mday)
		return d2;
	if(d1->tm_hour < d2->tm_hour)
		return d1;
	if(d1->tm_hour > d2->tm_hour)
		return d2;
	if(d1->tm_min < d2->tm_min)
		return d1;
	if(d1->tm_min > d2->tm_min)
		return d2;
	if(d1->tm_sec < d2->tm_sec)
		return d1;
	if(d1->tm_sec > d2->tm_sec)
		return d2;
	return d1;
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
	_tm 			f_date[_MAX_SENSOR_ID];
	int 			sfound[_NUMBER_OF_SENSORS];
	int 			i;
	int 			hit_cnt[_MAX_SENSOR_ID];

	printf("\nprocess_master v 0.0.4\r\n\n");
	printf("    opening %s\n",_SENSOR_MASTER_FILE_NAME);

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
		f_date[i].tm_year = 4000;
	}
	while(fread(&buffer, sizeof(buffer), 1, sensor_data) == 1)
	{
		rcnt++;
		printf("    %i records read\r", rcnt);
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
		f_date[buffer.sensor_id] = *fdate(&buffer.ts, &f_date[buffer.sensor_id]);
	}
	fclose(sensor_data);
	printf("\n\r    closing %s\n",_SENSOR_MASTER_FILE_NAME);
	// printf("\n%i records processed\n\r", rcnt);


	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0)
			avg_t[i] = sum_t[i] / hit_cnt[i];

	printf("\n   summery sensor values\n");
	printf("\n%6s%6s%7s%7s%17s%22s\n","ID","ave","max","min","first read","last read");
	printf("    --------------------------------------------------------------------------\n");
	for(i=0; i<_MAX_SENSOR_ID; i++)
		if(hit_cnt[i] > 0)
			printf("%6i %6.1f %6.1f  %5.1f %4i:%02i:%02i-%02i/%02i/%02i %4i:%02i:%02i-%02i/%02i/%02i\n", 
				i, avg_t[i], max_t[i], min_t[i], f_date[i].tm_hour, f_date[i].tm_min, f_date[i].tm_sec, f_date[i].tm_mon, f_date[i].tm_mday, f_date[i].tm_year,
				c_date[i].tm_hour, c_date[i].tm_min, c_date[i].tm_sec, c_date[i].tm_mon, c_date[i].tm_mday, c_date[i].tm_year);

	printf("\n %i records processed \n normal termination\n\n",rcnt);
  	return (0);
}

