#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>		//sleep
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <ctype.h> 		//isalnum, tolower
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"
#include "ipc.h"
#include "sys_dat.h"
#include "sch.h"
#include "cmd_defs.h"



void insert(char *bpt, char *cptr, int max){

}

void delete(char *bpt, char *cptr){

}

int main(void) {		
	char 			c, *work_buffer_ptr, *end_buff, *start_buff;
	static char 	work_buffer[15];

	/* initialize input buffer */
	work_buffer_ptr = work_buffer;
	start_buff = work_buffer;	
 	end_buff = (char *)((int)start_buff + 15);

	/* set up unbuffered io */
	fflush(stdout);
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

	printf("starting maim loop\n\r");

	while (1) {
		c = fgetc(stdin);
		switch (c) {
			case _NO_CHAR:	/* NOCR */ 
				break;
			case _CR:		/* CR */	
				printf("process buffer\n\r");
				break;
			case _DEL:		/* DEL */ 
				if (work_buffer_ptr > start_buff){
					fputc(_BS, stdout);
					fputc(' ', stdout);
					fputc(_BS, stdout);
					*work_buffer_ptr-- = '\0';
					*work_buffer_ptr = '\0';
				}
				break;
			case _ESC:		/* ESC */  
				c = fgetc(stdin);		// skip to next character
				c = fgetc(stdin);		// skip to next character
				switch(c){
					case 'A':	// up arrow
						printf("\n\rup arrow\n\r");
						printf("%s\n\r", work_buffer);
						continue;
						break;	
					case 'B':	// down arrow
						printf("\n\rdown arrow\n\r");	
						printf("%s\n\r", work_buffer);
						continue;
						break;		
					case 'C':	// right arrow
						if ((int)work_buffer_ptr < (int)end_buff){
							work_buffer_ptr++;
							printf("\033[1C");
						}	
						// printf("\n\rright arrow\n\r");
						// printf("%s\n\r", work_buffer);
						continue;
						break;
					case 'D':	// left arrow
						if ((int)work_buffer_ptr > (int)start_buff){
							work_buffer_ptr--;
							printf("\033[1D");
						}
						// printf("\n\rleft arrow\n\r");
						// printf("%s\n\r", work_buffer);
						continue;
						break;
					default:	// ESC
						printf("\nprocess escape\n\r");
						system("/bin/stty cooked");			//switch to buffered iput
						system("/bin/stty echo");			//turn on terminal echo
						printf("\f\n***normal termination\n\n\r");
						return 0;
						break;
				}
			default:	/* OTHER */ 
				if ((int)work_buffer_ptr < (int)end_buff){
					*work_buffer_ptr++ = c;	
					fputc(c, stdout);       				// echo char
				}
		}

	};



	// s_close(bbb);
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\n***normal termination -  but should not happen\n\n");
	return 0;
}
