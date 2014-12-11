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

Each channel can switch 120 volt 8 amp load.  The channel is controlled by a schedule for the current day of the week.  There can be a different schedule for each day of the week. Each channel has it own set of schedules, so for any one day of the week there are 8 (number of channels) schedules active.  

The software supports the creation of a library of schedules.  Any schedule can be assigned to any channel(s). 

####Hardware
* Parallax C3
* Parallax Digital IO Board
* BeagleBone Black
* DS3232 RTC
* MID400 AC line monitor

####Language
* C

####IDE
* Eclipse - cross compile BBB code
* SimpleIDE - cross compile C3 code

####Envirnoment
* Development machine - MacMini Linux Mint 17.0
* BeagleBone - Debian

####Hardware architecture


####Application architecture
The command processor is the most complex part of this project. The use of unbuffered input allows the application to mediately react to the press of the ECS key, but it requires that the application handle backspace/delete. The app maintains a buffer which matches the user's screen. When a CR is entered the screen buffer is passed to a state machine(char_fsm) which parses the screen buffer into a fifo stack of tokens. 

![character parser state diagram](state_diagrams/char_fsm-2-0-7.jpg?raw=true)

When the main event loop detects a non-empyt token stack it passes the stack to a second state machine (cmd_fsm) which processes the token stack. 

![command parser state diagram](state_diagrams/cmd_fsm.jpg?raw=true)

A third state machine handles communication with the C3. 

It runs on a BeagleBone Black and uses a comm
 

####Serial connection between the C3 and BeagleBone 

The C3 is running fdserial and UART1 is used on the BBB.  H
BBB					  C3
Tx P9_24  ----------> Rx 1
Rx P9_26  <---------- Tx 0

######Serial connection between C3 and DIOB
C3					DIOB
Din 4 ------------> Din      4 - dio, DIN
DATA_RLY     4 - dio, 
    5 - dio, SCLK_IN
    5 - dio, SCLK_RLY
    6 - dio, LOAD_INy
    7 - dio, LAT_RLY
    4 - dio, DATA_RLY
    5 - dio, SCLK_IN
    5 - dio, SCLK_RLY
    6 - dio, LOAD_IN
    7 - dio, LAT_RLY

The code was developed in c using SimpleIDE, Eclipse, SublimeText 3. The development platform is a Parallax C3 making use of flash memory and the SD card.  A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, the AQY212GH relays and terminals for the external connections are mounted on an additional board connected to the C3.

The application can be configured to drive 1 to 8 channels using the preprocessor variable *_NUMBER_OF_CHANNELS* in Pcon.h.  It can be configured to drive IO pins or to drive the Parallax Digital IO Board serial interface. The the preprocessor variable *_DRIVEN* in Pcon.h controls which interface is used. In one implementation I am driving AQY212GH PhotoMOS relays connected to 5 propeller IO pins.  The PhotoMOS relays are rated at 60 V AC/DC 1.1 A. These relays come in a 4-pin DIP package.  They work great to control 24 V zone valves. The second instance is driving a Parallax Digital IO board which can control 8, 120 VAC 8 A loads and accept input from 8 sensors. 
####Software environment:
C - Propgcc, Spin, Python, SimpleIDE, Sublime Text, Eclipse, Linux Debian (on the BBB), Cloud9, Linux Mint (cross compile)
####Hardware:
* C3 micro controller, Parallax  
* Digital IO Board (DIOB), Sharp solid state relays part# S202S02F, Parallax 
* ChronoDot real time clock module, based on a DS3231, adafruit 
* AQY212GH PhotoMOS relays, Newark
* MID400 AC Line Monitor, Newark 

####Command processor functions:
* name channels  
* manually control channel state  
* set channel control mode (manual, time, time & sensor)
* zero on time time for a channel
* load/save channel information to SD card  
* create and maintain schedules for each channel
* copy and paste schedules between days across channels
* load/save schedules to SD card (default file names)
* load/save schedules to a named file on the SD card
* display current time and date
* set date and time
* display schedules
* display system configuration information
* stop and start the control cogs
* set the real time clock
* reboot from EEPROM

####Command processor commands:
######schedule mode commands:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**copy** - copy the schedule for a channel and day to a buffer<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**paste** - paste the buffered schedule into a channel and day<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**delete** - delete the schedule for the channel and day<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**save** - save edited schedules data to default file<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**load** - load schedules from default file<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**file {file name} {load|save}** - save or load a named schedule file<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**edit** - edit the schedule for a specific channel and day<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**assign** - schedule to a (channel,day)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**delete** - delete a schedule record for the specified time<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**on** - set the schedule state to on for the specified time<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**off** - set the schedule state to off for the specified time<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**quit** - exit schedule edit mode<br />           
######channel mode commands:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**save** - save current channel data to default file<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**load** - load channel data from default file<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**name** - set the name of a channel<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**mode** - set the control mode of a channel, 0-manual, 1-time, 2-time & sensor<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**zero {channel #}** - set the on time accumlator for a channel to 0<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**on {channel #}** - force the control mode of a channel to manual and set the state to on<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**off {channel #}** - force the control mode of a channel to manual and set the state to off<br />
######system commands:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**system** - display system information<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**status** - display a formated dump of schedules, channel information and current state for all channels for all days<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**time** - display current time and date<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**set {YYYY}{MM}{DD}{N(day #)}{HH}{MM}{SS}** - set the real time clock<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**shutdown** - force all channels to manual control and turn them off, stop control cogs<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**start** - start control cogs<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**reboot** - reboot the system from EEPROM<br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**help(?)** - display list of valid commands<br />

Because the command processor is implemented by a state machine there is a lot of flexibility in the way tokens can be entered.  Entering a '?' will display the current state of the command fsm and a list of commands and tokens (INT for a integer and STR for a quoted string) that are valid in that state. ESC will clear input buffers and reset the state machines.   Tokens can be entered individually or strung together. Spaces, commas, colons and slashes will work as a delimiters. If the fsm requires additional information a prompt will be displayed, however the main loop will not wait for input.

![Command Processor FSM](FSM-docs/cmd_fsm.png?raw=true)

####Schedules:
A schedule is a list of times and corresponding states.  A channel that is controlled by time will be a list of times and states.  For example, a schedule of:

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

    0 - day light savings on
    1 - AC line power monitor MID400
    2 - serial port Rx
    3 - serail port Tx
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



 
