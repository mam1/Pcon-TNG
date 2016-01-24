/*
 * trace.h
 *
 *  Created on: Nov 1, 2014
 *      Author: mam1
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <stdbool.h>
#define _TRACE_FILE_NAME	"t02.trc"

int trace_on(char *,int *);
void trace(char *, char *, int, char *, char *, int);		//(trace filename, routine name, state, buffer, message, trace flag)
void strace(char *, char *, int);		//(trace filename, message, trace flag)



#endif /* TRACE_H_ */
