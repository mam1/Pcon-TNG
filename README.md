Pcon-TNG
========
- - - - - - - - - 
#####Random thoughts - under construction and not stable.
- - - - - - - - -
###Pcon - multi channel programmable controller
The state of up to 8 channels can be controlled by:

* time of day
* time of day and a sensor value
* cycle (seconds on, seconds off)
* manually
####Hardware:
* C3 micro controller, Parallax  
* Digital IO Board (DIOB), Sharp solid state relays part# S202S02F, Parallax 
* ChronoDot real time clock module, based on a DS3231, adafruit
* BeagleBone Black rev C, adafruit 
* AQY212GH PhotoMOS relays, Newark
* MID400 AC Line Monitor, Newark 

####Language
* C
####Envirnoment
* Development machine - MacMini Linux Mint 17.0
* BeagleBone Black (rev C)- Debian

####Channels
Each channel can switch 120 volt 8 amp load.  The channel is controlled by a schedule for the current day of the week.  There can be a different schedule for each day of the week. Each channel has it own set of schedules, so for any one day of the week there are 8 (number of channels) schedules active. The schedule for a picticular day is created my selecting a schedule "template" from the schedule libaray.  A template can be assigned to; a specific day for a specific channel, all days for a specific channel, all channels for a specific day. 

####Schedules:
A schedule  is a list of times and corresponding states.  A channel that is controlled by time will be a list of times and states.  For example, a schedule of:

* 1:00  on
* 13:00 off

will result in the channel turning on at 1:00AM and off at 1:00PM.  If the current time is between 13:00 and 24:00 or between 0:0 and 13:00 the channel will be off.  Between 1:00 and 13:00 it will be on. While a schedule of:

* 1:00  off
* 13:00 on 

will result in the channel being off between 1:00 - 13:00. It will be on at any time before 1:00 or after 13:00.

* 1:00  on
* 13:00 off
* 18:00 on

will result in the channel turning on at 1:00, off at 13:00 and on at 18:00.  If the current time is between 1:00 and 13:00 the channel will be on, between 13:00 and 18:00 it will be off, between 18:00 and 0:0 it will be on and between 0:0 and 13:00 it will also be on. 

The maximum number of records for a schedule is configured by setting the preprocessor variable *_MAX_SCHEDULE_RECS*.  Schedules are stored on the SD card. 

####Application development
The code was developed in c using SublimeText. The development machine is a MacMini running Mint 17.  The code is cross compiled on the mac.  The C3 is connected to the development machine via USB.  The development machine mounts a nsf share from the BeagleBone.

The command processor is the most complex part of this project. The use of unbuffered input allows the application to mediately react to the press of the ECS key, but it requires that the application handle backspace/delete. The app maintains a buffer which matches the user's screen. When a CR is entered the screen buffer is passed to a state machine(char_fsm) which parses the screen buffer into a fifo stack of tokens. 

![character parser state diagram](state_diagrams/char_fsm.jpg?raw=true)

When the main event loop detects a non-empty token stack it passes the stack to a second state machine (cmd_fsm) which processes the token stack. 

![command parser state diagram](state_diagrams/cmd_fsm.jpg?raw=true)

A third state machine handles communication with the C3. 

It runs on a BeagleBone Black and uses a comm
 

###Hardware architecture

![hardware layout](state_diagrams/hardware.jpg?raw=true)


A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, headers and terminals for the external connections are mounted on an additional board connected to the C3. 

####Serial connection between the C3 and BeagleBone 
The C3 is running fdserial and UART1 is used on the BBB.  H
BBB					  C3
Tx P9_24  ----------> Rx 1
Rx P9_26  <---------- Tx 0

######Serial connection between C3 and DIOB
C3    DIOB (serial)
pin 4  DATA_RLY
pin 5  SCLK_IN
pin 5  SCLK_RLY
pin 6  LOAD_IN
pin 7  LAT_RLY

####Architecture:
A schedule is a vector of 32 bit unsigned integers. The length is configured by the preprocessor variable *_MAX_SCHEDULE_RECS* .  These are fixed length vectors. I have an alternate implementation using linked lists but his approach is much simpler and has less chance of memory leaks. Since I am already using xmmc size is not a big deal.

The first 32 bits of the vector contain the number of active records in the schedule. The following 32 bit "records" are interpreted as:

* bit 31 (high bit) .... state (0=off, 1=on)
* bit 30-16 ............ key (number of minutes past midnight, 0-1440)
* bit 15-0 ............. sensor value (0-65536)

The schedules for each (day,channel) tuple are combined into one contiguous vector of unsigned integers. The schedule for a particular tuple is accessed via pointer offsets. 

The control part of the application uses 2 cogs, "rtc.cogc" and "dio.cogc".  The rtc cog talks to the DS3231, converts BCD to decimal and updates a time/date buffer in hub memory.  The rtc cog contains i2c bit banging code; first - because the DS3231 requires clock stretching if the code is running in a cog and  second - the library i2c routines are too large. I discovered the need for clock stretching  by developing the bit banging code on the xmmc cog, have it running fine, moving it to a cog and it dies.  It must be the difference in execution speed.  Once I implemented the complete i2c spec (add clock stretching) it runs fine on a cog.  The library i2c routines do not support clock stretching, but that is not important as they are too large to run from a cog.

The dio cog reads the time from the buffer in hub memory.  Once a minute the dio cog creates a control byte.  Each bit controls the state of the corresponding channel. The bits are set based on the current time, the schedule for the channel and the control information for the channel. The dio cog can be forced to update by use of a flag in hub memory. The control byte is sent to either the Parallax Digital IO Board or the PhotoMos relays depending on the application configuration.

The schedule and control information are stored on a SD card and loaded into hub memory at initialization or on command.  

The complex part of the application is the command processor.  XMMC is required because of the code size.  It uses a finite state machine (fsm) to parse the input character stream into tokens and a second fsm to process the tokens.  This type of command processor is probably inappropriate for a micro controller, however no one is paying me anymore so I can do what I want. 

The command processor loops checking to see if a character has been typed. Input buffering has been disabled so the read is non blocking.

**If a character is present**, unless it is an ESC, it is passed to the first state machine (char_fsm). An ESC will clear all buffers and reset both state machines.  The first fsm, char_fsm parses the input stream into tokens and pushes them on to FIFO stack.  A CR will cause char_fsm to pass the stack of tokes to the command processor.  The command processor pops tokens off the stack and feeds them to a second fsm, cmd_fsm until the stack is empty. Then the command processor lets the cms_fsm know there is no more input then continues the main loop.  While cmd_fsm is processing a token stack the main loop is waiting, however, the the control cogs are running independently and are not affected.  They continue to control the channels based the the real time. 
**If a character is not found** the code checks to see if the the cogs have sent any messages.  If the dio cog changes the state of a channel it sets a flag to let the main loop know to update the channel info stored on disk.
![Main Event Loop](flow_charts/main_loop.png?raw=true)
####SD Files:
Schedules and persistent channel information (name, control mode, state) are stored in files on a SD card. The files are loaded when the code is initialized or reset. They can also be loaded or saved on command.  The files are alway closed after use so the SD card can be replaced to make copies or change behavior.  The file names are generated using the following preprocessor variables:

    #define _FILE_SET_ID            "NNN"
    #define _F_PREFIX               "SSS"
    #define _F_SCHEDULE_SUFIX       ".sch"
    #define _F_CHANNEL_SUFIX        ".ch"

In the following format:

    channel information <SSS><NNN>.ch
    schedules           <SSS><NNN>.sch

####Propeller Pins:

    0 - serial port Rx
    1 - serail port Tx
    2 - 
    3 - 
    4 - dio, DIN
    4 - dio, DATA_RLY
    5 - dio, SCLK_IN
    5 - dio, SCLK_RLY
    6 - dio, LOAD_IN
    7 - dio, LAT_RLY
    8 - SPI
    9 - SPI, SPI_MOSI
    10 - SPI, SPI_MISO
    11 - SPI, SPI_CLK
    12 - composite video
    13 - composite video
    14 - composite video
    15 - toggle Port B header
    16-23 - VGA 
    24 - audio PWM
    25 - SPI
    26 - PS/2_DATA
    27 - PS/2_CLOCK. 
    28 - i2c
    29 - i2c
    30 - USB
    31 - USB

####BeagleBone Balck Pins



 
