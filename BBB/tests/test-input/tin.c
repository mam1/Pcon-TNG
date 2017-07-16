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
	char 			c, *work_buffer_ptr, *end_buff, *start_buff, *move_ptr;
	char 			*input_ptr;
	static char 	work_buffer[_INPUT_BUFFER_SIZE];
	char 			ring_buffer[_CMD_BUFFER_DEPTH][_INPUT_BUFFER_SIZE];	// char array[NUMBER_STRINGS][STRING_MAX_SIZE];
	int 			rb_in_idx, rb_out_idx;
	int 			mv, i;

	/* initialize input buffer */
	work_buffer_ptr = work_buffer;
	start_buff = work_buffer;	
 	end_buff = (char *)((int)start_buff + _INPUT_BUFFER_SIZE);

 	/* initialize ring buffer & indexs*/
 	for(i=0;i<_CMD_BUFFER_DEPTH;i++)
 		memset(&ring_buffer[i][0],'\0',_INPUT_BUFFER_SIZE);
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
				strcpy(&ring_buffer[rb_in_idx++][0], work_buffer);
				if(rb_in_idx > _CMD_BUFFER_DEPTH - 1)
					rb_in_idx = 0;
				rb_out_idx = rb_in_idx;	
				printf("\n\rprocess buffer  {%s}\n\r> ", work_buffer);
				work_buffer_ptr = work_buffer;
				memset(work_buffer, '\0', sizeof(work_buffer));
				memset(&ring_buffer[rb_in_idx][0], '\0', _INPUT_BUFFER_SIZE);
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
					mv = work_buffer_ptr - input_ptr;
					input_ptr--;
					*input_ptr = '*';
					while(input_ptr < work_buffer_ptr){
						*input_ptr = *(input_ptr+1);
						input_ptr++;
					}
					*work_buffer_ptr-- = '\0';
					// printf("\033[s");	// save cursor position	       			
					// printf("\r> %s", work_buffer);
					// printf("\033[u");	// Restore cursor position
					// printf("\033[1C");	// move cursor right

				
					// printf("\r\033[K");
					printf("\033[K");	// Erase to end of line
					printf("\r> %s", work_buffer);
					while(mv > 0){
							printf("\033[1D");	// move cursor left
							mv--;
						}
				}
				break;
			case _ESC:		/* ESC */  
				c = fgetc(stdin);		// skip to next character
				c = fgetc(stdin);		// skip to next character
				switch(c){
					case 'A':	// up arrow
						// printf("\n\rup arrow\n\r");

						if(rb_out_idx > 0)
							rb_out_idx--;
						else
							rb_out_idx = rb_in_idx - 1;

						strcpy(work_buffer,&ring_buffer[rb_out_idx][0]);
						if(rb_out_idx >= rb_in_idx)
							rb_out_idx = 0;
						printf("\r");
						printf("\033[K");	// Erase to end of line
						printf("\r> %s", work_buffer);
						continue;
						break;	
					case 'B':	// down arrow
						// printf("\n\rdown arrow\n\r");
						rb_out_idx++;
						if(rb_out_idx > rb_in_idx)
							rb_out_idx = rb_in_idx;
						
						strcpy(work_buffer,&ring_buffer[rb_out_idx][0]);	
						printf("\r");
						printf("\033[K");	// Erase to end of line
						printf("\r> %s", work_buffer);
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
						printf("ring buffer dump: rb_in_idx {%i}, rb_out_idx {%i}\n\r",rb_in_idx, rb_out_idx);
						for(i=0;i<_CMD_BUFFER_DEPTH;i++)
							printf("    {%s}\n\r", &ring_buffer[i][0]);


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

						// fputc('\033[s', stdin);		// save cursor position	
						move_ptr = work_buffer_ptr++;
						move_ptr++;
						*move_ptr-- = '\0';
						while(move_ptr > input_ptr){
							*move_ptr = *(move_ptr - 1);
							move_ptr--;
						}
						*input_ptr++ = c;

						mv = work_buffer_ptr - input_ptr;
						// printf("\033[s");	// save cursor position	
						// printf("\033[s");	// save cursor position	
						printf("\r> %s", work_buffer);
						// printf("\033[u");	// Restore cursor position
						// printf("\033[u");	// Restore cursor position
						// fputc('\033[u', stdin);	// Restore cursor position
						// printf("\033[3C");	// move cursor right
						while(mv > 0){
							printf("\033[1D");	// move cursor left
							mv--;
						}

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
