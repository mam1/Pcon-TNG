/*
 * trace.c
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#include <stdio.h>
#include "shared.h"
#include "typedefs.h"
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

void trace1(char *name, char *caller, char *message){	//(trace filename, calling module, pointer to system data sturcture)
	FILE *tracef;
	tracef = fopen(name, "a");
	fprintf(tracef,"%s: %s\n",caller,message);
	fclose(tracef);
	return;
}

void trace2(char *name, char *caller, _SYS_DAT *s){
	FILE *tracef;
	tracef = fopen(name, "a");
	fprintf(tracef,"%s: major_version = %i, minor_version=%i, minor_revision=%i, channels=%i, sensors=%i, commands=%i, states=%i\n",
		    caller,
		    s->config.major_version,
		    s->config.minor_version,
		    s->config.minor_revision,
		    s->config.channels,
		    s->config.sensors,
		    s->config.commands,
		    s->config.states);
	fclose(tracef);
	return;
}

void trace3(char *name, char *caller, char *message, int number){
	FILE *tracef;
	tracef = fopen(name, "a");
	fprintf(tracef,"%s: %s %i\n",caller,message,number);
	fclose(tracef);
	return;
}