#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			6
#define _minor_version 			0
#define _minor_revision 		0

/* shared bone-C3 commands */
#define _PING         	27
#define _ACK           	6
#define _WRITE_CMD		1
#define _READ_CMD		2
#define _WRITE_SCH		3
#define _PUSH_STATS		4

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	8
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		10
#define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
// #define _SCHEDULE_BUFFER		128

#define	_BYTES_PER_INT 				4
#define _BYTES_PER_SCHEDULE 	_SCHEDULE_SIZE * _BYTES_PER_INT
#define _BYTES_PER_DAY 			_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* general */
#define _DAYS_PER_WEEK 				7

#endif