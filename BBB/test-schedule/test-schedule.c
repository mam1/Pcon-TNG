
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "shared.h"
// #include "ipc.h"
#include "Pcon.h"
#include "shared.h"
// #include "bitlit.h"
// #include "PCF8563.h"
#include "typedefs.h"
#include "sys_dat.h"
#include "sch.h"
// #include "gpio.h"
// #include "led.h"
// #include "schedule.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *c_mode[4] = {"manual", "  time", "   t&s", " cycle"};

/*******************************************/

int main (void) {

    _S_TAB      sch;
    _SYS_DAT2   sys_data;
    FILE 		*sys_file;
    int         day;
    int         channel;
    int         hour;
    int         minute;
    int         state;
    int         temp;
    int         humid;
    int         sensor_id;
    char 		*fname = "test_sys.dat";


    printf("\n **** starting\n");
    printf("  size of schedule %i bytes\n", sizeof(sch));

    sys_file = sys_open(fname,&sys_data);
    sys_load(fname,&sys_dat);


	
    /* create schedule records */
    printf("  enter channel number > ");
    scanf("%i", &channel);
    printf("  enter day number > ");
    scanf("%i", &day);
    printf("  enter hour > ");
    scanf("%i", &hour);
    printf("  enter minute > ");
    scanf("%i", &minute);
    printf("  enter state > ");
    scanf("%i", &state);
    printf("  enter temp > ");
    scanf("%i", &temp);
    printf("  enter humid > ");
    scanf("%i", &humid);

    printf("\n  creating schedule record for channel %i, day %i, time %02i:%02i, state %i, temp %i humid %i\n\n",
        channel,day, hour, minute, state, temp, humid);

    add_sch_rec2(&sch,day,channel,hour,minute,state,temp,humid);
    add_sch_rec2(&sch,day,channel,hour+5,minute,state,temp+1,humid);
    add_sch_rec2(&sch,day,channel,hour-2,minute,state,temp+2,humid);
    add_sch_rec2(&sch,day,channel,hour-7,minute,state,temp+3,humid);
    dump_schedule(&sch,day,channel);

    printf("delete day %i channel %i %02i:%02i\n",day, channel, hour, minute);
    del_sch_rec2(&sch,day,channel,hour,minute);
    dump_schedule(&sch,day,channel);

    temp = 99;
    printf("update temp to %i for day %i channel %i %02i:%02i\n",temp, day, channel, hour-2, minute);
    up_sch_rec_temp(&sch, day, channel, hour-2, minute, temp);
    dump_schedule(&sch,day,channel);

    printf("\n **** ending\n");
    return (0);
}

