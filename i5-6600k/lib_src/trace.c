/*
 * trace.c
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

/* trace version info */
#define _MAJOR_VERSION    0
#define _MINOR_VERSION    0
#define _MINOR_REVISION   0

#include <stdio.h>

void trace3(char *name, char *caller, char *message, int number){
	FILE *tracef;
	tracef = fopen(name, "a");
	fprintf(tracef,"%s: %s %i\n",caller,message,number);
	fclose(tracef);
	xreturn;
}
