#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			6
#define _minor_version 			0
#define _minor_revision 		9

#define TRACE on

/* semaphore key */
#define _SEM_KEY 				1234321

/* sensor parameteres */
#define _NUMBER_OF_SENSORS		8

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	16
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		10
#define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
// #define _SCHEDULE_BUFFER		128

#define	_BYTES_PER_INT 			4
#define _BYTES_PER_SCHEDULE 	_SCHEDULE_SIZE * _BYTES_PER_INT
#define _BYTES_PER_DAY 			_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* gpio enable list */
#define _NUMBER_OF_GPIOS	16
#define _GPIOS              66,67,69,68,45,44,26,115,46,27,65,61,60,48,49,20	
#define _HEADERS			 8, 8, 8, 8, 8, 8, 8, 9, 8, 8, 8, 8, 9, 9, 9, 9
#define _PINS  				 7, 9, 8,10,11,12,14,27,16,17,18,26,12,15,23,41	 		

/* map P8 header pins */
#define _LED_1			7	
#define _LED_2			9		
#define _LED_3			8			
#define _LED_4			10
#define _DIOB_DIN 		11
#define _DIOB_DATA_RLY	11
#define _DIOB_SCLK_IN	12
#define	_DIOB_SCLK_RLY	12
#define	_DIOB_LAT_RLY	14
#define	_DIOB_LOAD_IN	27
#define _R1_CAPE		16
#define _R2_CAPE		17
#define _R3_CAPE		18
#define _R4_CAPE		26
 
/* map P9 header pins  */
#define _R5_CAPE		12
#define _R6_CAPE		15
#define _R7_CAPE		23
#define _R8_CAPE		41

/* general */
#define _DAYS_PER_WEEK 	7
#define _ON 			1
#define _OFF 			0

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif