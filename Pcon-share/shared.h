
/* shared codes, bone and prop */
#define PING         	27
#define ACK           	6
#define WRITE_CMD		1
#define READ_CMD		2
#define WRITE_SCH		3
#define PUSH_STATS		4

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	8
#define _CHANNEL_NAME_SIZE		20
#define _MAX_SCHEDULE_RECS		10
#define _INTS_PER_SCHEDULE			_MAX_SCHEDULE_RECS + 1
#define _BYTES_PER_SCHEDULE			(_INTS_PER_SCHEDULE) * 4 	//a uint32_t contains the number of active records followed by vector of uint32_ts
#define _BYTES_PER_WORKING_SET		_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE
#define _DAYS_PER_WEEK 			7