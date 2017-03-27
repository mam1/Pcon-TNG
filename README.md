
## PCON-TNG
------
### *** under construction and not stable ***
------
These are my notes on developing a multi channel programmable HVAC controller. The current iteration of the system is comprised of several ESP8266 modules, a Beagle Bone Black (BBB), A BBB Cape from Waveshare with a real time clock, a BBB custom Control Cape built for this project and a Digital IO Board from Parallax. The hardware supports 16 channels:

	channel number
    	0-7 ............ low voltge/current channels (e.g. controlling zone valves) 
    	8-15 ........... high voltage/current channels (e.g. controling 120 volt AC motors)

Temperature and humidity data is collected by ESP8266 modules. The ESP8266 modules read HDT22 sensors and use a wireless connection to post the data to the cloud (ThingSpeak) and to an Apache sever running on the BBB. The BBB logs the data from the ESP8266 modules. A process running on the BBB decides if a channel should be on or off.  A channel can be controlled by:

* time of day
* time of day and a sensor value
* cycle (seconds on, seconds off)
* manually

The BBB Control Cape (part of this project) uses PhotoMOS relays for controlling 24 volt zone valves.  PhotoMOS are resistant to inrush current (due to phase shift) and eliminate the need for snubber circuits as long as they are operated within the ratings. Furthermore, use of PhotoMOS® decreases the mounting area requirements, resulting in more compact programmable controllers. LEDs are used to indicate the state of the 16 channels.  There is a 16 position header connected to the photoMOS relays for directly switching 8, 24 volt, zone valves. A second 20 position header provides TTL signals for controlling a Parallax Digital IO Board (DIOB). The DIOB can control 8, 120VAC 8A loads.

The BBB provides a state machine driven user interface to configure channels, build and maintain schedules, query sensor and channel data, manually control channel states, etc.
- - - - - - - - -
#### Hardware
  * **BeagleBone Black Rev C - 4GB Flash, adafruit**
  * **BBB Control Cape**
    *  1 - Beaglebone Black expansion board, Waveshare
    *  8 - AQY212GH PhotoMOS relays, Newark
    * 16 - red LEDs
    *  2 - 100 ohm, 8 resistor DIP
    *  1 - 2 x 8 header, zone valve control
    *  1 - 2 x 5 header, DIOB control
            * Digital IO Board (DIOB), Sharp solid state relays part# S202S02F, Parallax
  * **BBB Misc Cape, Waveshare**
    * PCF8563 RTC
  * **Wireless Sensors**
    * ESP8266 NodeMCU Dev Board, R2, Electro Dragon
    * DHT22 temperature/humidity sensor, Electro Dragon
    * 0.96” 128*64 OLED Display (i2c), Electro Dragon

##### BeagleBone GPIO assignments
      header  pin #   gpio    use
    -------------------------------------------------------------------
    LEDs on WaveShare Misc Cape
        P8      7      66     Led 1 on WaveShare Misc Cape
        P8      8      67     Led 2 on WaveShare Misc Cape
        P8      9      69     Led 3 on WaveShare Misc Cape
        P8     10      68     Led 4 on WaveShare Misc Cape


     DIOB serial interface
        P8     11      45     DIN on DIOB serial header 		connected to DATA_RLY on cape
        P8     11      45     DATA_RLY on DIOB serial header 	connected to DIN on cape
        P8     12      44     SCLK_IN on DIOB serial header 	connected to SCLK_RLY on cape
        P8     12      44     SCLK_RLY on DIOB serial header 	connected to SCLK_IN on cape
        P8     14      26     LOAD_IN on DIOB serial header
        P8     15      47     LAT_RLY on DIOB serial header

	Relays on my cape       
        P8     16      46     Relay 1 on proto Cape
        P8     17      27     Relay 2 on proto Cape
        P8     18      65     Relay 3 on proto Cape
        P8     26      61     Relay 4 on proto Cape
        P9     12      60     Relay 5 on proto Cape
        P9     15      48     Relay 6 on proto Cape
        P9     23      49     Relay 7 on proto Cape
        P9     41      20     Relay 8 on proto Cape



###### I2c connection between the BBB and PCF8563 real time clock on the Misc Cape 
The WaveShare cape is jumbered to use i2c2 on the bone.

##### Serial connection between BBB and DIOB
The serial interface (2x5 header) reduces the number of pins required to control the Digital I/O Board by serially shifting data to/from the board over a synchronous serial interface. Whereas full control requires 16 I/O pins using the parallel interface, the serial interface can provide full control with as few as 4 I/O pins.

The pins labeled DATA_RLY, SCLK_RLY, LAT_RLY and /OE_RLY are connected to a 74HC595 serial to parallel shift register. These pins have the following functions: DATA_RLY is serial data going out to the shift register. This data is sent MSB first (8 bits) synchronously with the SCLK_RLY pin. Once the data has been shifted out the LAT_RLY pin must be pulsed to latch the data onto the outputs. /OE_RLY must be LOW in order for the 74HC595 to drive the relays. This can be tied to VSS or controlled via an I/O pin.

The pins labeled DIN, SCLK_IN and LOAD_IN are connected to a 74HC165 parallel to serial shift register. These pins have the following functions: DIN is serial data from the shift register sent synchronously with a clock pulse on SCLK_IN generated by the host microcontroller. Before data is shifted in the current contents of the inputs must be loaded. This is accomplished by pulsing the LOAD_IN line from HIGH to LOW and then back to HIGH. This line must remain HIGH during shift operations and must only go LOW during a LOAD operation.

The DIN line has a 1K resistor in series to allow sharing of the 74HC165 data line with the 74HC595 DATA_RLY line. Connecting these lines together and also connecting SCLK_RLY to SCLK_IN reduces the number of I/O pins required by the microcontroller down to four (4). In this configuration LAT_RLY and LOAD_IN must have their own I/O pins and /OE_RLY can be connected to VSS. This circuit provides a half-duplex system where you can access one shift register at a time. For full duplex operation the DIN and DATA_RLY lines must be separate and for independent operation the SCLK_RLY and SCLK_IN must also be separate.

#### Development Environment:

All code is stored on github.  The repository is cloned on two development machines.  At one site I have a MacMini running Linux Mint 17.x and at the other I have have an iMac running OS X 10.10.x . The Bone is connected to my network. I can access it from either site using ssh. I use rsync to move the binaries from the development machines to the Bone. 

When cross compiling for the Bone on OS X I am using a tool chain I got from http://will-tm.com/cross-compiling-mac-os-x-mavericks. The only thing I needed to do was to create a case sensitive partition before copying the files from the .dmg file.  On the Linux box I tried installing installing a cross compile tool chain using the Mint software manager and experienced a lot of problems. After spending a lot of time dealing with missing files I gave up and installed the linaro-arm-linux-gnueabihf-4.9-2014.09_linux tool chain and it worked on the first try.

#### Language
* C - BBB
* Lua - ESP8266

#### Envirnoment
* Development machines - MacMini Linux Mint 17.x - OS X 10.11.x
* BeagleBone Black (rev C)- Debian
* ESP8266 (nodeMCU dev board r2) - nodeMCU firmware 

#### Channels and Schedules 
A channel is controlled by a schedule. There is a different schedule for each day of the week. Each channel has it own set of schedules. Channels 0-7 can switch a 60 volt 1.1 amp loads.  Channels 8 -15 can switch 120 volt 8 amp loads.  Each channel can be set manually to on or off.  Channels and also be controlled by time of day, i.e. on at 8:00 off at 14:30 or sensor value and time.  When a channel is controlled by time and sensor once a minute the remote sensor value is compared to the value stored in the schedule for current time and day of the week. If the actual values exceeds the schedule value the channel is turned off when. If the schedule values is less than the actual value the channel is turned on.  

##### Schedules:
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

A schedule is implemented as a vector of 32 bit unsigned integers. The length is configured by the preprocessor variable *_MAX_SCHEDULE_RECS*.  These are fixed length vectors. 

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






####Application Architecture
The command processor is the most complex part of this project. The use of unbuffered input allows the application to mediately react to the press of the ECS key, but it requires that the application handle backspace/delete. The app maintains a buffer which matches the user's screen. When a CR is entered the screen buffer is passed to a state machine(char_fsm) which parses the screen buffer into a fifo stack of tokens. 

![character parser state diagram](./support_docs/state_diagrams/char_fsm.jpg?raw=true "character parser FSM")


When the main event loop detects a non-empty token stack it passes the stack to a second state machine (cmd_fsm) which processes the token stack. 

![command parser state diagram](./support_docs/state_diagrams/cmd_fsm.jpg?raw=true "command parser FSM")

A third state machine handles communication with the C3. 

It runs on a BeagleBone Black and uses a comm

A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, headers and terminals for the external connections are mounted on an additional board connected to the C3. 


 
