#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			6
#define _minor_version 			0
#define _minor_revision 		2

/* sensor parameteres */
#define _NUMBER_OF_SENSORS		10

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	16
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		10
#define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
// #define _SCHEDULE_BUFFER		128

#define	_BYTES_PER_INT 				4
#define _BYTES_PER_SCHEDULE 	_SCHEDULE_SIZE * _BYTES_PER_INT
#define _BYTES_PER_DAY 			_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* map leds to P8 header pins */
#define _LED_1		7	
#define _LED_2		9		
#define _LED_3		8			
#define _LED_4		10	

/* general */
#define _DAYS_PER_WEEK 				7
#define _ON 						1
#define _OFF 						0



#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif