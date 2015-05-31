#ifndef SHARED_H
#define SHARED_H


// ======<< Ascii control Characters >>=====
#define      _SOH  0x01
#define      _STX  0x02
#define      _ETX  0x03
#define      _EOT  0x04
#define      _ACK  0x06
#define      _NAK  0x15
#define      _SYN  0x16

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

/* frame types */
#define _SCHEDULE_F     1 // replace a working schedule with schedule in frame, send ack to sender
#define _CHANNEL_F      2   // replace channel data with channel data in frame, send channel data back to sender
#define _TIME_F         3   // set real time clock to the time/date in frame, send ack to sender
#define _PING_F     4   // request for ping, send ping data in frame back to sender 
#define _ACK_F         5 //
#define _REBOOT_F      6 // reboot the C3, program load from EEPROM 

#define _MODEMDEVICE "/dev/ttyO1"	//Beaglebone Black serial port


#endif