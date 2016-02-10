
#include <stdio.h>
#include <unistd.h>   //sleep
#include <stdint.h>   //uint_8, uint_16, uint_32, etc.
#include <ctype.h>    //isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "shared.h"
#include "ipc.h"
#include "Pcon.h"
#include "shared.h"
#include "bitlit.h"
#include "PCF8563.h"
// #include "gpio.h"
// #include "led.h"
#include "schedule.h"

/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *c_mode[4] = {"manual", "  time", "   t&s", " cycle"};


int main (void) {
	uint32_t 			sch[4] = {0,0,0,0};
	int 				key[3];
	int 				i, s, tkey;

	printf("\n **** starting\n");
	sch[0] = 3;

	key[0] = make_key(1, 10);
	key[1] = make_key(1, 20);
	key[2] = make_key(1, 30);

	for(i=0;i<3;i++)
		printf("key %i is %i\n",i, key[i]);

	s = 1;
	printf("start loop\n");
	for(i=0;i<3;i++){
		put_key(&sch[i+1], key[i]); 		// load key into a schedule record
		put_state(&sch[i+1], s);         // load state into a schedule record
		printf("state <%i> loaded into rec with key <%i>\n",s,key[i]);
		if(s == 1)
			s = 0;
		else
			s = 1;
	}
	printf("schedule record initialized\ndump schedule:\n");

	for(i=1;i<4;i++){
		printf("    key = %i, state = %i\n", key[i-1], get_s(sch[i]));
	}
	printf("\ntest keys against schedule\n");

	tkey = make_key(1, 10);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(1, 20);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(1, 30);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(0, 32);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(1, 12);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(1, 25);
	printf("  test key <%i>  -  ", tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key

	tkey = make_key(1, 32);
	printf("  test key <%i>  -  ",tkey);
	printf("test returned <%i>\n",test_sch(sch, tkey));		 //return state for key




	printf("\n **** ending\n");
	return (0);
}

