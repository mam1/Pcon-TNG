
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
char *mode[4] = {"manual", "  time", "   t&s", " cycle"};

/*******************************************/

int main (void) {

    int         	hour;
    int         	minute;
    int         	state = 0;
    int         	temp = 0;
    int         	humid = 0;
    _TMPL_DAT 		sch;

    printf("\n **** starting\n");
    printf("  size of schedule %i bytes\n", sizeof(sch));



    /* create base schedule record */
    printf("  enter hour > ");
    scanf("%i", &hour);
    // printf("  enter state > ");
    // scanf("%i", &state);
    // printf("  enter temp > ");
    // scanf("%i", &temp);
    // printf("  enter humid > ");
    // scanf("%i", &humid);

    minute = 30;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

    minute = minute - 5;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

    minute = minute + 20;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

    minute = minute - 7;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

    minute = minute + 10;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);
  
    minute = minute + 2;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

        minute = minute - 8;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

        minute = minute - 8;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

  	humid = 99;
    printf("\n  changeing schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);

        minute = 57;
    printf("\n  creating schedule record for time %02i:%02i, state %i, temp %i humid %i\n\n",
        hour, minute, state, temp, humid);
    add_tmpl_rec(&sch,hour,minute,state,temp,humid);
    dump_template(&sch);


    minute = 25;
    printf("\ndelete  %02i:%02i\n", hour, minute);
    del_tmpl_rec(&sch,hour,minute);
    dump_template(&sch);

    // temp = 99;
    // printf("update temp to %i for day %i channel %i %02i:%02i\n",temp, day, channel, hour-2, minute);
    // up_sch_rec_temp(&sch, day, channel, hour-2, minute, temp);
    // dump_template(&sch);

    printf("\n **** ending\n");
    return (0);
}

