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


int main(void) {		
	char 			c, *work_buffer_ptr, *end_buff, *start_buff, *move_ptr, *end_ptr;
	char 			*input_ptr;
	static char 	work_buffer[_INPUT_BUFFER_SIZE];
	static char 	ring_buffer[_INPUT_BUFFER_SIZE][_CMD_BUFFER_DEPTH];
	static int 		rb_in_idx, rb_out_idx;

	/* initialize input buffer */
	work_buffer_ptr = work_buffer;
	start_buff = work_buffer;	
 	end_buff = (char *)((int)start_buff + _INPUT_BUFFER_SIZE);

 	/* initialize ring buffer */
 	rb_in_idx  = 0;
 	rb_out_idx = 0;

	/* set up unbuffered io */
	fflush(stdout);
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

	printf("starting main loop\n\n\r> ");
	input_ptr = work_buffer_ptr;
	while (1) {
		c = fgetc(stdin);
		switch (c) {
			case _NO_CHAR:	/* NOCR */ 
				break;
			case _CR:		/* CR */	
				printf("\n\rprocess buffer  {%s}\n\r> ", work_buffer);
				work_buffer_ptr = work_buffer;
				 memset(work_buffer, '\0', sizeof(work_buffer));
				input_ptr = work_buffer_ptr;
				break;
			case _DEL:		/* DEL */
				if(work_buffer_ptr <= start_buff)
					break; 

				if(input_ptr == work_buffer_ptr){	// no arrow keys in play
					*work_buffer_ptr-- = '\0';
					*work_buffer_ptr = '\0';
					input_ptr = work_buffer_ptr;
					printf("\033[2D");	// move cursor left
					printf("\033[K");	// Erase to end of line
					printf("\033[s");	// save cursor position	       			
					printf("\r> %s", work_buffer);
					printf("\033[u");	// Restore cursor position			
				}
				else {

					// while(input_ptr > work_buffer_ptr){
					// 	*input_ptr = *(input_ptr + 1);
					// 	input_ptr++;
					// }

					// *work_buffer_ptr-- = '\0';
					// printf("\033[s");	// save cursor position	       			
					// printf("\r> %s", work_buffer);
					// printf("\033[u");	// Restore cursor position
					// printf("\033[1C");	// move cursor right

				
					// printf("\r\033[K");
					// printf("\r> %s", work_buffer);
					// printf("\033[u");	//Restore cursor position
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
						// input_ptr = work_buffer_ptr;
						if (input_ptr < work_buffer_ptr){
							input_ptr++;
							printf("\033[1C");	// move cursor right
						}	
						continue;
						break;
					case 'D':	// left arrow
						// input_ptr = work_buffer_ptr;
						if (input_ptr > start_buff){
							input_ptr--;
							printf("\033[1D");	// move cursor left
						}
						continue;
						break;
					default:	// ESC
						printf("\n\rprocess escape\n\r");
						system("/bin/stty cooked");			//switch to buffered input
						system("/bin/stty echo");			//turn on terminal echo
						printf("\r\n***normal termination\n\n\r");
						return 0;
						break;
				}
			default:	/* OTHER */ 
				if (work_buffer_ptr < end_buff){		// room to add character ?
					if(input_ptr == work_buffer_ptr){	// no arrow keys in play
						*work_buffer_ptr++ = c;
						input_ptr = work_buffer_ptr;	       			
						printf("\r> %s", work_buffer);
					}

					else{		// cursor is not at the end of the input buffer
						move_ptr = work_buffer_ptr++;
						move_ptr++;
						*move_ptr-- = '\0';
						while(move_ptr > input_ptr){
							*move_ptr = *(move_ptr - 1);
							move_ptr--;
						}
						*input_ptr++ = c;
						printf("\033[s");	// save cursor position	       			
						printf("\r> %s", work_buffer);
						printf("\033[u");	// Restore cursor position
						printf("\033[1C");	// move cursor right

					}
		}

	}
}



	// s_close(bbb);
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\n***normal termination -  but should not happen\n\n");
	return 0;
}
