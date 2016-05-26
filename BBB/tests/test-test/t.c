
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

	_IPC_DAT 			ipc;
	int 				i;

    // _S_TAB       sch;
    _TMPL_DAT    t_sch1[20]; // , c_sch2

printf("\n *** testing ***\n\n  before load c_sch1\n");
dump_template(&ipc.sys_data.t_data[0]);
printf("adding a template record\n");
add_tem_rec2(&ipc.sys_data.t_data[0], 2, 4, 1, 99, 25);
add_tem_rec2(&ipc.sys_data.t_data[0], 3, 4, 0, 89, 55);
add_tem_rec2(&ipc.sys_data.t_data[0], 1, 4, 1, 65, 75);
printf("\nafter load c_sch1\n");
dump_template(&ipc.sys_data.t_data[0]);

printf("**********************\n");

dump_template(t_sch1);

for(i=0;i<ipc.sys_data.schlib_index;i++)
	t_sch1[i] = ipc.sys_data.t_data[i];

dump_template(t_sch1);


    return 0;
}

