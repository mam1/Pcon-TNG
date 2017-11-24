
## PCON-TNG
------
### *** under construction and not stable ***
------
These are my notes on developing a multi channel programmable HVAC controller. The current iteration of the system is comprised of several ESP8266 modules, a BeagleBone Black (BBB), a BBB custom Control Cape built for this project and Digital IO Board(s) from Parallax. The system supports 16 channels.

A daemon (Dcon) is started as part of the boot process of the BeagleBone. The daemon decides if a channel should be on or off.  It does this by looking at a memory mapped file (ipc.dat).  If the file does not exist the daemon creates and initializes it.  The memory mapped file creates a persistent space in virtual memory allowing processes to communicate.  It is maintained by a user interface process (Pcon).  Pcon is only active when a user is interacting with the system.  Temperature and humidity data is collected by ESP8266 modules. The ESP8266 modules read HDT22 sensors and use a wireless connection to post the data to the cloud (ThingSpeak) and to an Apache sever running on the BBB. A CGI on the BBB makes the data available to the daemon by updating shared memory and posts the data to a SQL database.

A channel can be controlled by:

* time of day
* time of day and a sensor value
* manually

The BBB Control Cape (part of this project) maps 16 BeagleBone gpio pins to 2 2x10 pin headers.  Each header can drive 1 Parallax Digital IO board (DIOB).  It also connects 4 gpio pins to a heart beat display on the cape.

The Pcon application runs on the BBB.  It interacts with the daemon using shared memory.  Pcon provides a state machine driven user interface to configure channels, build and maintain schedules, query sensor and channel data, manually control channel states, etc.
- - - - - - - - -
#### Hardware
  * **BeagleBone Black Rev C - 4GB Flash, adafruit**
  * **BBB Control Cape**
    *  2 - 2 x 10 headers, DIOB control
            * Digital IO Board (DIOB), Sharp solid state relays part# S202S02F, Parallax
    *  4 - heart beat leds
  * **Wireless Sensors**
    * ESP8266 NodeMCU Dev Board, R2, Electro Dragon
    * DHT22 temperature/humidity sensor, Electro Dragon
    * 0.96” 128*64 OLED Display (i2c), Electro Dragon

##### BeagleBone GPIO assignments
                           {header,pin,gpio}
    * #define _CHAN0          {8,27,86}
    * #define _CHAN1          {8,28,88}
    * #define _CHAN2          {8,29,87}
    * #define _CHAN3          {8,30,89}
    * #define _CHAN4          {8,31,10}
    * #define _CHAN5          {8,32,11}
    * #define _CHAN6          {8,33,9}
    * #define _CHAN7          {8,34,81}
    * #define _CHAN8          {8,35,8}
    * #define _CHAN9          {8,36,80}
    * #define _CHAN10         {8,37,78}
    * #define _CHAN11         {8,38,79}
    * #define _CHAN12         {8,39,76}
    * #define _CHAN13         {8,40,77}
    * #define _CHAN14         {8,41,74}
    * #define _CHAN15         {8,42,75}
    * #define _HB0            {8,43,72}
    * #define _HB1            {8,44,73}
    * #define _HB2            {8,45,70}
    * #define _HB3            {8,46,71}


##### Parallel connection between BBB and DIOB(s)


#### Development Environment:

All code is stored on github.  The repository is cloned on two development machines.  At one site I have a Linux box running Mint 18.x and at the other I have have an iMac running OS X 10.10.x . The Bone is connected to my network. I can access it from either site using ssh. I use rsync to move the binaries from the development machines to the Bone. 

When cross compiling for the Bone on OS X I am using a tool chain I got from http://will-tm.com/cross-compiling-mac-os-x-mavericks. The only thing I needed to do was to create a case sensitive partition before copying the files from the .dmg file.  On the Linux box I tried installing installing a cross compile tool chain using the Mint software manager and experienced a lot of problems. After spending a lot of time dealing with missing files I gave up and installed the linaro-arm-linux-gnueabihf-4.9-2014.09_linux tool chain and it worked on the first try.

#### Language
* C - BBB
* Lua - ESP8266

#### Envirnoment
* Development machines - core 5i Linux Mint 18.x - OS X 10.11.x
* BeagleBone Black (rev C)- Debian, jessie
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

#### Application Architecture
The ESP8266 modules run independently of other system components.  Each module posts its sensor readings to an Apache web server running on the bone.  A CGI (Scon) loads the readings into a shared memory space that can be accessed by the command processor and the daemon.  The state of each channel is reset once every minute by the daemon process (Dcon), running on the bone.  The daemon does not require the command processor (Pcon) to be active.  The command processor controls the daemon by setting values in a shared memory space implemented with memory mapped file io.  

##### Command Processor
![Pcon flow chart](./BBB/UI/Pcon_flow.jpg?raw=true "Pcon flow chart")

The command processor is the most complex part of this project. The use of unbuffered input allows the application to mediately react to the press of the ECS key, but it requires that the command processor handle the arrow and delete keys. The command processor maintains a buffer which matches the user's screen. The process loops waiting for a character to be detected.  ESC, Del, up_arrow, down_arrow, left_arrow and right_arrow are handled directly.  An ESC will clear all buffers and reset both state machines.  The delete and arrow commands work as they do on the command line. Any other character is passed to the first state machine (char_fsm).  It parses the input stream into tokens and pushes them on to FIFO stack.  A CR will cause char_fsm to pass the stack of tokes to the command processor.  

The command processor pops tokens off the stack and feeds them to a second fsm (cmd_fsm).  It takes some action based on the token.  If more input is required it is popped off the stack, if present, or the user is prompted. This continues until the stack is empty. Then the command processor lets the cms_fsm know there is no more input then continues the main loop, waiting for user input. 

![character parser state diagram](./support_docs/state_diagrams/char_fsm.jpg?raw=true "character parser FSM")


When the main event loop detects a non-empty token stack it passes the stack to a second state machine (cmd_fsm) which processes the token stack. 

![command parser state diagram](./support_docs/state_diagrams/cmd_fsm.jpg?raw=true "command parser FSM")

 


 
