#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			6
#define _minor_version 			0
#define _minor_revision 		8

/* semaphore key */
#define _SEM_KEY 				1234321

/* sensor parameteres */
#define _NUMBER_OF_SENSORS		8

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	6
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		10
#define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
// #define _SCHEDULE_BUFFER		128

#define	_BYTES_PER_INT 			4
#define _BYTES_PER_SCHEDULE 	_SCHEDULE_SIZE * _BYTES_PER_INT
#define _BYTES_PER_DAY 			_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* map P8 header pins */
#define _LED_1			7	
#define _LED_2			9		
#define _LED_3			8			
#define _LED_4			10
#define _DIOB_DIN 		11
#define _DIOB_DATA_RLY	12
#define _DIOB_SCLK_IN	14
#define	_DIOB_SCLK_RLY	15
#define	_DIOB_LOAD_IN	16
#define	_DIOB_LAT_RLY	17
 
/* map P9 header pins to PhotoMOS relays */
#define _R1_CAPE	12
#define _R2_CAPE	13
#define _R3_CAPE	15
#define _R4_CAPE	23
#define _R5_CAPE	25
#define _R6_CAPE	27
#define _R7_CAPE	30
#define _R8_CAPE	41

/* general */
#define _DAYS_PER_WEEK 				7
#define _ON 						1
#define _OFF 						0



#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif