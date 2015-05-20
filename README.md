Pcon-TNG
========
- - - - - - - - - 
###*** under construction and not stable ***

These are my notes on developing a multi channel programmable controller application which is distributed across a BeagleBone Black and a Parallax C3. The BeagleBone handles the user interface and the C3 deals with interacting with the physical devices. Initial versions of this application run on a single C3. It works, however, the complexity of the command processor is pushing the limits of the C3. Moving the command processor to the BeagleBone allows the user interface to be more user friendly.
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
 
#####2c connection between the C3 and DS3231 real time clock module
A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, headers and terminals for the external connections are mounted on an additional board connected to the C3.

#####Serial connection between the C3 and BeagleBone
The C3 is running fdserial on pins 0 & 1. They are connected through 1k resistors to UART1 on the BBB.

BBB ------------------ C3

Tx P9_24 ----------> Rx 1

Rx P9_26 <---------- Tx 0

#####Serial connection between C3 and DIOB
C3 ------------------ DIOB (serial)

pin 4 ..................... DATA_RLY

pin 5 ..................... SCLK_IN

pin 5 ..................... SCLK_RLY

pin 6 ..................... LOAD_IN

pin 7 ..................... LAT_RLY

####Development Environment:
I want to be able to share data structures and possibly some code between the C3 and the Bone. Since there are GCC cross compiles available for both platforms that would seem to make C the obvious choice for a language. The C3 does not have an OS, it is pretty much bare metal. The Bone comes with Debian (Wheezy) installed.

I work on this project from two locations. At one site I have a MacMini running Linux Mint 17.1 and at the other I have have an iMac running OS X 10.10.1. There is a clone of this directory on both machines. The Bone is connected to my network. I can access it from either site using ssh. I use rsync to move the binaries from the development machines to the Bone. The C3 is physically connected (USB) to the Linux machine. Currently I need to be on the Linux machine to load the C3. I need to figure out how to load the C3 from the Bone. The Pcon-BBB directory holds the Bone code and Pcon-C3 holds the C3 code. My Makefiles check the OS and do the appropriate things, allowing me to build from either site.
OS X
C3

I tried to build propgcc. I put a little effort into it before I gave up and downloaded the OS X version of SimpleIDE which installed the propgcc binaries. SimpleIDE worked with no tweaking. SimpleIDE is somewhere between Eclipse and Sublime Text in functionality. It is aimed a specific platform which does not make it work well for my application. I have used Eclipse for J2EE development. Once you get it configured, it is very powerful, but it seemed to be to heavy for this project. I decided to go with Sublime Text, which meant I needed to learn a little about make files.
BBB

When cross compiling for the Bone on OS X I am using a tool chain i got from http://will-tm.com/cross-compiling-mac-os-x-mavericks. The only thing I needed to do was to create a case sensitive partition before copying the files from the .dmg file.
Linux
C3

Even after installing the "correct" version of textifno I had no luck building propgcc, although it did get much further after I downgraded textinfo. I installed the Linux version of SimpleIDE to get the binaries and to give me a tool to work with the C3 while I tried to figure out make file.
BBB

I tried installing installing a cross compile tool chain using the Mint software manager and experienced a lot of problems. After spending a lot of time dealing with missing files I gave up and installed the xxxx tool chain and it worked on the first try.

####Language
* C

####Envirnoment
* Development machine - MacMini Linux Mint 17.0
* BeagleBone Black (rev C)- Debian
* C3 -  bare metal

######see the project wiki for more information on the development environment

####Channels
Each channel can switch 120 volt 8 amp load.  The channel is controlled by a schedule for the current day of the week.  There can be a different schedule for each day of the week. Each channel has it own set of schedules, so for any one day of the week there are 8 (number of channels) schedules active. The schedule for a picticular day is created my selecting a schedule "template" from the schedule libaray.  A template can be assigned to; a specific day for a specific channel, all days for a specific channel, all channels for a specific day. 

####Application Architecture
The command processor is the most complex part of this project. The use of unbuffered input allows the application to mediately react to the press of the ECS key, but it requires that the application handle backspace/delete. The app maintains a buffer which matches the user's screen. When a CR is entered the screen buffer is passed to a state machine(char_fsm) which parses the screen buffer into a fifo stack of tokens. 

![character parser state diagram](state_diagrams/char_fsm.jpg?raw=true)

When the main event loop detects a non-empty token stack it passes the stack to a second state machine (cmd_fsm) which processes the token stack. 

![command parser state diagram](state_diagrams/cmd_fsm.jpg?raw=true)

A third state machine handles communication with the C3. 

It runs on a BeagleBone Black and uses a comm

A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, headers and terminals for the external connections are mounted on an additional board connected to the C3. 

#####Schedules:
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

A schedule is implemented as a vector of 32 bit unsigned integers. The length is configured by the preprocessor variable *_MAX_SCHEDULE_RECS* .  These are fixed length vectors. I have an alternate implementation using linked lists but his approach is much simpler and has less chance of memory leaks. Since I am already using xmmc size is not a big deal.

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

####BeagleBone Black Pins



 
