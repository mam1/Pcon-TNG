/*
 * Pcon.c
 *
 *  Created on: Nov 28, 2014
 *      Author: mam1
 */


#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.

#include "Pcon.h"

int main(int argc, char *argv[]){

	sleep(1);
	disp_sys();

	return 0;
}

void disp_sys(void) {
	printf("*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version, _minor_revision);
	return;
}

