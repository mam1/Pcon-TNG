#ifndef _SHARE_H_
#define _SHARE_H_

/* trace options */
// #define _ATRACE			// trace all functions 
// #define _PTRACE			// trace Pcon 
// #define _DTRACE			// trace Dcon
// #define _STRACE			// trace Scon
// #define _FTRACE			// trace library functions

/* version info */
// #define _MAJOR_VERSION		10
// #define _MINOR_VERSION 		4
// #define _MINOR_REVISION		0

/* file names */
#define _FILE_NAME_SIZE 		50
// #define _CGI_DATA_FILE_PREFIX	"/media/TOSHIBA/sensor"		// cgi generated log of sensor data
// #define _CGI_DATA_FILE_SUFIX	".dat"							// cgi generated log of sensor data
#define _SENSOR_DATA_FILE_NAME		"/media/data/sensors/sensor.dat" 		// USB drive collecting sensor readingss
#define _CGI_LOG_FILE_NAME			"/media/data/logs/CGI.log"				// log file name
// #define _SYSTEM_FILE_NAME			"/home/Pcon-data/sys.dat"			// system data file
#define _IPC_FILE_NAME				"/home/Pcon-data/ipc.dat"   			// memory mapped ipc file
#define _IPC_FILE_BACKUP_NAME		"/home/Pcon-data/ipc.bkup"   			// ipc backup file 
#define _TRACE_FILE_NAME			"/hmedia/data/trace/Pcon-trace.dat" 	// trace file
#define _DAEMON_LOG 				"/media/data/logs/daemon.log" 			// daemon log file name
#define _SENSOR_MASTER_FILE_NAME    "/media/Raid/sensor logs/master.dat"	// cumulative history of sensor readings
#define _PID_FILE_NAME 				"/run/Dcon.pid"

/* semaphore key */
#define _SEM_KEY 				1234321

/* character parser fsm */
#define _CHAR_TOKENS     		5
#define _CHAR_STATES     		4
#define _MAX_TOKEN_SIZE			128
#define _TT_INT					-1
#define _TT_STR					-2
#define _TT_NULL				-3
#define _TT_UNREC				-4

/* command parser fsm */
#define _CMD_TOKENS     		39
#define _CMD_STATES     		35 

/* key codes */
#define _ESC        27
#define _SPACE      32
#define _COLON      58
#define _SLASH      47
#define _COMMA      44
#define _BS         8
#define _DEL		127
#define _QUOTE      34
#define _CR         13
#define _NL 		10
#define _FF 		12
#define _EOF		0
#define _NO_CHAR    255
#define _DELIMITER	28	
#define _UPA		124

/* buffers */
#define _INPUT_BUFFER_SIZE		60
#define _TOKEN_BUFFER_SIZE		_INPUT_BUFFER_SIZE
#define _PROMPT_BUFFER_SIZE		500
#define _CMD_BUFFER_DEPTH 		10 
#define _TRACE_BUFFER_SIZE 		128		
#define _SCREEN_BUFFER_SIZE		_INPUT_BUFFER_SIZE + _PROMPT_BUFFER_SIZE
/* sensor parameteres */
#define _NUMBER_OF_SENSORS		20
#define _MAX_GROUP_NAME_SIZE 	6
// #define _MAX_SENSORS			25
#define _SNAME_SIZE 			20
#define _SDESCRIPTION			_MAX_TOKEN_SIZE
#define _MAX_SENSOR_ID			99

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	16
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		50
#define _MAX_SCHEDULES			24
#define _ALL_CHANNELS			99
#define _ALL_DAYS				99

/* template library parameters */
#define _MAX_TEMPLATES			10
#define _TEMPLATE_NAME_SIZE		40

// #define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
// #define _SCHEDULE_BUFFER		128

#define	_BYTES_PER_INT 			4
#define _BYTES_PER_SCHEDULE 	_SCHEDULE_SIZE * _BYTES_PER_INT
#define _BYTES_PER_DAY 			_NUMBER_OF_CHANNELS * _BYTES_PER_SCHEDULE 	

/*	gpio pin assignment {header,pin,gpio}  	*/
#define _CHAN0			{8,16,46}
#define _CHAN1			{8,17,27}
#define _CHAN2			{8,18,65}
#define _CHAN3			{8,26,61}
#define _CHAN4			{9,12,60}
#define _CHAN5			{9,15,48}
#define _CHAN6			{9,23,49}
#define _CHAN7			{9,41,20}
#define _CHAN8			{8,41,20}
#define _CHAN9			{8,36,80}
#define _CHAN10			{8,37,78}
#define _CHAN11			{8,38,79}
#define _CHAN12			{8,39,76}
#define _CHAN13			{8,40,77}
#define _CHAN14			{8,41,74}
#define _CHAN15			{8,42,75}
#define _HB0 			{8,7,66}
#define _HB1 			{8,9,67}
#define _HB2 			{8,8,69}
#define _HB3 			{8,10,68}

/* general */
#define _DAYS_PER_WEEK 	7
#define _ON 			1
#define _OFF 			0
#define _TRUE 			1
#define _FALSE			0

/* macros */
#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif	//_SHARE_H_
