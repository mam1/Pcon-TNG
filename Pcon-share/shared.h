#ifndef _SHARE_H_
#define _SHARE_H_

/* version info */
#define _major_version 			4
#define _minor_version 			1
#define _minor_revision 		2

// ======<< Ascii control Characters >>=====
#define      _SOH  0x01
#define      _STX  0x02
#define      _ETX  0x03
#define      _EOT  0x04
#define      _ACK  0x06
#define      _NACK  0x15
#define      _SYN  0x16

/* frame types */
#define _SCHEDULE_F     1 // replace a working schedule with schedule in frame, send ack to sender
#define _CHANNEL_F      2 // replace channel data with channel data in frame, send channel data back to sender
#define _TIME_F         3 // set real time clock to the time/date in frame, send ack to sender
#define _PING_F     	4 // request for ping, send ping data in frame back to sender 
#define _ACK_F         	5 //
#define _NACK_F         6 //
#define _REBOOT_F      	7 // reboot the C3, program load from EEPROM 

/* shared bone-C3 commands */
#define _PING         	27
#define _WRITE_CMD		1
#define _READ_CMD		2
#define _WRITE_SCH		3
#define _PUSH_STATS		4
#define _RECEIVE_SYS_DAT	5
#define _BLOCK			7
#define _INT 			8
#define _SYSTEM			9

/* serial communication bone / prop */
#define _MODEMDEVICE   "/dev/ttyO1"	//Beaglebone Black serial port
#define _BAUD			115200
#define _MAX_PACKET		253
#define _TIMEOUT_USEC	0
#define _TIMEOUT_SEC 	30
#define _RETRY			6

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

/* character parser fsm */
#define _CHAR_TOKENS     		5
#define _CHAR_STATES     		4
#define _MAX_TOKEN_SIZE			128

/* command parser fsm */
#define _CMD_TOKENS     		34
#define _CMD_STATES     		22 

#define _MAX_SCHLIB_SCH			20
#define _MAX_TMPLLIB_SCH		20

/* buffers */
#define _INPUT_BUFFER_SIZE		128
#define _TOKEN_BUFFER_SIZE		60
#define _PROMPT_BUFFER_SIZE		200

/* files */
#define _BLOCKING				512

// #define _TIME_STRING_BUFFER     40
// #define _VCMD_BUFFER            128

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
#define _DELIMITER	42	//asterisk

/* general */
#define _DAYS_PER_WEEK 				7
#define      False 0
#define      True 1

#endif
