/*
 * trace.h
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <stdbool.h>
#include "typedefs.h"


int trace_on(char *,int *);
void trace(char *, char *, int, char *, char *, int);	// (trace filename, routine name, state, buffer, message, trace flag)
void strace(char *, char *, int);						// (trace filename, message, trace flag)
void trace1(char *name, char *caller, char *message);	// (trace filename, calling module, message)
void trace2(char *name, char *caller, _SYS_DAT2 *s);		// (trace filename, calling module, pointer to system data)
void trace3(char *name, char *caller, char *message, int number); // (trace filename, calling module, message, int)

#endif /* TRACE_H_ */
