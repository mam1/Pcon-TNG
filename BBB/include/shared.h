#ifndef _SHARE_H_
#define _SHARE_H_

/* trace options */
#define _ATRACE			// trace all functions 
#define _PTRACE			// trace Pcon 
// #define _DTRACE			// trace Dcon
// #define _STRACE			// trace Scon
#define _FTRACE			// trace library functions

/* version info */
// #define _MAJOR_VERSION		10
// #define _MINOR_VERSION 		4
// #define _MINOR_REVISION		0

/* file names */
#define _FILE_NAME_SIZE 		50
// #define _CGI_DATA_FILE_PREFIX	"/media/TOSHIBA/sensor"		// cgi generated log of sensor data
// #define _CGI_DATA_FILE_SUFIX	".dat"							// cgi generated log of sensor data
#define _SENSOR_LOG_FILE_NAME		"/media/CGI-sensor-log/sensor.dat"
#define _CGI_LOG_FILE_NAME			"/home/Pcon-data/cgi.log"
#define _SYSTEM_FILE_NAME			"/home/Pcon-data/sys.dat"		// system data file
#define _IPC_FILE_NAME				"/home/Pcon-data/ipc.dat"   	// memory mapped ipc file
#define _IPC_FILE_BACKUP_NAME		"/home/Pcon-data/ipc.bkup"   	// ipc backup file 
#define _TRACE_FILE_NAME			"/home/Pcon-data/trace.dat" 	// trace file
#define _LOG_FILE_NAME				"/media/CGI sensor log/log.dat"		// log file name
#define _DAEMON_LOG 				"/home/Pcon-data/daemon.log" 	// daemon log file name
#define _SENSOR_MASTER_FILE_NAME    "/media/Raid/SensorMaster/master.log"

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
#define _CMD_TOKENS     		38
#define _CMD_STATES     		29 

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
#define _INPUT_BUFFER_SIZE		128
#define _TOKEN_BUFFER_SIZE		60
#define _PROMPT_BUFFER_SIZE		500
#define _CMD_BUFFER_DEPTH 		10 		

/* sensor parameteres */
#define _NUMBER_OF_SENSORS		10
#define _MAX_SENSORS			25
#define _SNAME_SIZE 			10
#define _SDESCRIPTION			50

/* channel parameters */
#define _NUMBER_OF_CHANNELS 	16
#define _CHANNEL_NAME_SIZE		20

/* schedule parameters */
#define _SCHEDULE_NAME_SIZE		40
#define _MAX_SCHEDULE_RECS		50
#define _MAX_SCHEDULES			24

/* template library parameters */
#define _MAX_TEMPLATES			10
#define _TEMPLATE_NAME_SIZE		40

// #define _SCHEDULE_SIZE			_MAX_SCHEDULE_RECS + 1
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
#define _TRUE 			1
#define _FALSE			0

/* macros */
#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)



#endif	//_SHARE_H_
