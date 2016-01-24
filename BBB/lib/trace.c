/*
 * trace.c
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#include <stdio.h>
#include "trace.h"

int trace_on(char *name, int *flag) {
	FILE *tracef;

	tracef = fopen(name, "w");		//make sure that there is an empty log file
	if (tracef != NULL){
		printf(" trace file <%s> opened\n", name);
		fprintf(tracef,"\n************************************************************************\nstart trace\n");
		fclose(tracef);
		return 0;
	}
	else{
		printf(" can't open trace file <%s>\ntrace disabled\n", name);
		*flag = false;
		return 1;
	}
}

void trace(char *name, char *rname, int state, char *buf, char *message,int flag){		//(trace filename, routine name, state, buffer, message, trace flag)
	char			mess_buf[128];
	FILE *tracef;

	if(flag == false)
		return;

    sprintf(mess_buf, "%s: %s\n  current state <%d>\n  input_buffer <%s>", rname, message, state, buf);
	tracef = fopen(name, "a");
	fprintf(tracef,"%s\n",mess_buf);
	fclose(tracef);
	return;
}


void strace(char *name, char *message,int flag){		//(trace filename, message, trace flag)
	FILE *tracef;
	if(flag == false)
		return;

	tracef = fopen(name, "a");
	fprintf(tracef,"%s\n",message);
	fclose(tracef);
	return;
}

