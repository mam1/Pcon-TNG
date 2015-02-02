#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			2
#define _minor_version 			1
#define _minor_revision 		2

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
#define _SCHEDULE_BUFFER	


#define _BYTES_PER_SCHEDULE			(_INTS_PER_SCHEDULE) * 4 	//a uint32_t contains the number of active records followed by vector of uint32_ts
#define _BYTES_PER_WORKING_SET		_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE

/* general */
#define _DAYS_PER_WEEK 			7
#define	_BYTES_PER_INT 				4

#endif