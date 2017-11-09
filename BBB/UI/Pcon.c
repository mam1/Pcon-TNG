
/*****************************************/
/* This is the main Pcon.c program file  */
/* BeagleBone Black implementation		 */
/*										 */
/*  									 */
/*****************************************/

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

/******************************** globals **************************************/
int				trace_flag;							//control program trace
int 			ipc, bkup; 							//ipc file flags
int 			bbb;								//UART1 file descriptor
_CMD_FSM_CB  	cmd_fsm_cb;							//cmd_fsm control block
_IPC_DAT 		*ipc_ptr; 							//ipc data
void			*data = NULL;						//pointer to ipc data
char           	ipc_file[] = {_IPC_FILE_NAME};  			//name of ipc file
uint8_t 		cmd_state, char_state;				//fsm current state
char 			tbuf[_TOKEN_BUFFER_SIZE];
key_t 			skey = _SEM_KEY;
int 			semid;
unsigned short 	semval;
struct sembuf 	wait, signal;
union semun {
	int 				val;        //used for SETVAL only
	struct semid_ds 	*buf; 		//for IPC_STAT and IPC_SET
	uint8_t 			*array;     //used for GETALL and SETALL
};
union 			semun dummy;
struct sembuf sb = {0, -1, 0};  /* set to allocate resource */

int 		cmd_buffer_push_index, cmd_buffer_pop_index;
char 		cmd_buffer[_CMD_BUFFER_DEPTH][_INPUT_BUFFER_SIZE + 1]; // array to hold multiple single arrays of characters


/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *mode[4] = {"manual", "  time", "sensor", " cycle"};

/***************************** support routines ********************************/

/* prompt for user input */
void prompt(int s) {

	// printf("************** prompt called\r\n");
	// printf("********** record count before call to build_prompt %i\r\n", cmd_fsm_cb.w_template_buffer.rcnt);
	build_prompt(&cmd_fsm_cb);
	printf("%s <%i> ", cmd_fsm_cb.prompt_buffer, s);
	// printf("************** prompt returning\r\n");
	return;
}

/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/

int main(void) {
	uint8_t 		c;       			//character typed on keyboard
	int				char_state;			//current state of the character processing fsm
	int 			prompted = false;	//has a prompt been sent
	int 			i;
	int 			fd;					//file descriptor for ipc data file

	char 			*work_buffer_ptr, *end_buff, *start_buff, *move_ptr;
	char 			screen_buf[_SCREEN_BUFFER_SIZE], *cursor_ptr;
	char 			*input_ptr, *hptr;
	static char 	work_buffer[_INPUT_BUFFER_SIZE];
	char 			ring_buffer[_CMD_BUFFER_DEPTH][_INPUT_BUFFER_SIZE];	// char array[NUMBER_STRINGS][STRING_MAX_SIZE];
	int 			rb_in_idx, rb_out_idx;
	int 			mv;
	int  			escape;

	char 			*ppp;

	/*********************** setup console *******************************/

	printf("Pcon %d.%d.%d starting\n\r", _MAJOR_VERSION_Pcon, _MINOR_VERSION_Pcon, _MINOR_REVISION_Pcon);

	/************************* setup trace *******************************/


	/************************ initializations ****************************/

	/* set up file mapped shared memory for inter process communication */
	ipc_sem_init();								// setup semaphores
	semid = ipc_sem_id(skey);					// set semaphor id

	/* set up shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = data; 							// overlay data with _IPC_DAT data structure
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* setup control block pointers */
	cmd_fsm_cb.ipc_ptr = ipc_ptr;					 	//set pointer to shared memory
	cmd_fsm_cb.sys_ptr = &(ipc_ptr->sys_data);		 	//set pointer to system data in shared memory
	cmd_fsm_cb.ssch_ptr = &ipc_ptr->sys_data.sys_sch; 	//set pointer to active shecule in shared memory
	cmd_fsm_cb.wsch_ptr = &cmd_fsm_cb.w_sch;		 	//set pointer to working schedule

	printf(" System version (app) %d.%d.%d\n\r", _MAJOR_VERSION_system, _MINOR_VERSION_system, _MINOR_REVISION_system);
	printf(" System version (shr mem) %d.%d.%d\n\r", ipc_ptr->sys_data.config.major_version, ipc_ptr->sys_data.config.minor_version, ipc_ptr->sys_data.config.minor_revision);

	if (sys_comp(&(ipc_ptr->sys_data.config))) 
	{
		printf("*** the system configuration in shared memory and in the application are different\n update shared memory? (y)|(n) > ");
		if (getchar() == 'y') 
		{
			ipc_ptr->sys_data.config.major_version = _MAJOR_VERSION_system;
			ipc_ptr->sys_data.config.minor_version = _MINOR_VERSION_system;
			ipc_ptr->sys_data.config.minor_revision = _MINOR_REVISION_system;
			ipc_ptr->sys_data.config.channels = _NUMBER_OF_CHANNELS;
			ipc_ptr->sys_data.config.sensors = _NUMBER_OF_SENSORS;
			ipc_ptr->sys_data.config.commands = _CMD_TOKENS;
			ipc_ptr->sys_data.config.states = _CMD_STATES;
			c = fgetc(stdin);			// get rid of trailing CR
		}
		else 
		{
			c = fgetc(stdin);			// get rid of trailing CR
			printf("*** application terminated\n");
			exit(1);
		}
	}

	/* load working schedule from system schedule */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	cmd_fsm_cb.w_sch = ipc_ptr->sys_data.sys_sch;
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* initialize working sensor name and description */
	cmd_fsm_cb.w_sen_dat.name[0] = '\0';
	cmd_fsm_cb.w_sen_dat.description[0] = '\0';
	cmd_fsm_cb.w_sen_dat.description[1] = '\0';

	/* initialize state machines */
	cmd_fsm_reset(&cmd_fsm_cb); 			//initialize the command processor fsm
	char_fsm_reset();
	char_state = 0;							//initialize the character fsm

	/* initialize input buffer */
	memset(work_buffer, '\0', sizeof(work_buffer));
	work_buffer_ptr = work_buffer;
	start_buff = work_buffer;
	input_ptr = work_buffer;
	end_buff = (char *)((int)start_buff + _INPUT_BUFFER_SIZE);
	escape = false;

	/* initialize ring buffer & indexs*/
	for (i = 0; i < _CMD_BUFFER_DEPTH; i++)
		memset(&ring_buffer[i][0], '\0', _INPUT_BUFFER_SIZE);
	rb_in_idx  = 0;
	rb_out_idx = 0;

	/* set up unbuffered io */
	fflush(stdout);
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);
	escape = false;

	printf("initializations complete\r\nenter ? for a list of commands\r\n\n");

	/* set initial prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, "enter a command");

	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/



	while (1) 
	{
		/* check the token stack */
		while (pop_cmd_q(cmd_fsm_cb.token))
		{
			cmd_fsm(&cmd_fsm_cb);   	//cycle cmd fsm until queue is empty
			prompted = false;
		}
		if (prompted == false) {				//display prompt if necessary
			prompted = true;
			prompt(cmd_fsm_cb.state);
		}
		c = fgetc(stdin);	// read the keyboard
		switch (c) 
		{
/* NOCR */	case _NO_CHAR:	
				break;

/* ESC */ 	case _ESC:		 
				c = fgetc(stdin);		// skip to next character
				c = fgetc(stdin);		// skip to next character
				switch(c)
				{
	/* up arrow */	case 'A':	
						if(rb_out_idx > 0)
							rb_out_idx--;
						else
							rb_out_idx = rb_in_idx - 1;
						strcpy(work_buffer,&ring_buffer[rb_out_idx][0]);
						if(rb_out_idx >= rb_in_idx)
							rb_out_idx = 0;

						printf("\r");
						// printf("\033[1A");				// move cursor up one line
						prompt(cmd_fsm_cb.state);		// display user prompt

						printf("%s", work_buffer);		// print work_buffer
						printf("\033[K");				// Erase to end of line
						work_buffer_ptr = work_buffer;
						while(*work_buffer_ptr++);		// move pointer to end of line
						input_ptr = --work_buffer_ptr;
						continue;
						break;	
	/* down arrow */case 'B':	
						rb_out_idx++;
						if(rb_out_idx > rb_in_idx)
							rb_out_idx = rb_in_idx;
						strcpy(work_buffer,&ring_buffer[rb_out_idx][0]);	
						printf("\r");
						printf("\033[K");	// Erase to end of line
						printf("\r> %s", work_buffer);
						continue;
						break;		
	/* right arrow */case 'C':	
						if (input_ptr < work_buffer_ptr){
							input_ptr++;
							printf("\033[1C");	// move cursor right
						}	
						continue;
						break;
	/* left arrow */case 'D':	
						if (input_ptr > start_buff){
							input_ptr--;
							printf("\033[1D");	// move cursor left
						}
						continue;
						break;
	/* ESC */		default:
						escape = true;
						while (pop_cmd_q(cmd_fsm_cb.token)); 	//empty command queue
						cmd_fsm_reset(&cmd_fsm_cb);				//reset command fsm
						memset(work_buffer, '\0', sizeof(work_buffer));	//clean out work buffer
						work_buffer_ptr = work_buffer;
						char_fsm_reset();						//reset char fsm
						prompted = false;						//force a prompt
						strcpy(cmd_fsm_cb.prompt_buffer, "\r\ncommand processor reset\n\renter a command");
						break;
				}

/* CR */	case _CR:

				if(work_buffer_ptr != start_buff) // skip null lines
				{
					strcpy(&ring_buffer[rb_in_idx++][0], work_buffer);
					if(rb_in_idx > _CMD_BUFFER_DEPTH - 1)
						rb_in_idx = 0;
					rb_out_idx = rb_in_idx;	
				}

				printf("\r\n");						// move cursor to next line
				*work_buffer_ptr++ = _CR;			// load the CR into the work buffer
				*work_buffer_ptr++ = '\0';			// load the NULL into the work buffer
				work_buffer_ptr = work_buffer;		// reset pointer
				char_fsm_reset();					// reset char_fsm

				while (*work_buffer_ptr != '\0')	// send characters to char_fsm
					char_fsm(char_type(*work_buffer_ptr), &char_state, work_buffer_ptr++); 

				work_buffer_ptr = work_buffer;		// reset pointer
				input_ptr = work_buffer_ptr;		// reset pointer
				cursor_ptr = screen_buf;			// reset pointer

				memset(work_buffer, '\0', sizeof(work_buffer));
				memset(screen_buf, '\0', sizeof(screen_buf));

				strcpy(screen_buf,cmd_fsm_cb.prompt_buffer);
				printf("%s",screen_buf);

				memset(&ring_buffer[rb_in_idx][0], '\0', _INPUT_BUFFER_SIZE);
				break;
/* DEL */	case _DEL:		
				if(work_buffer_ptr <= start_buff)
					break; 

				if(input_ptr == work_buffer_ptr){	// no arrow keys in play
					*work_buffer_ptr-- = '\0';
					*work_buffer_ptr = '\0';
					input_ptr = work_buffer_ptr;

					printf("\033[1D");	// move cursor left
					printf("\033[K");	// Erase to end of line
					printf("\033[s");	// save cursor position	       			
					printf("\r> %s", work_buffer);
					printf("\033[u");	// Restore cursor position			
				}
				else {
					mv = work_buffer_ptr - input_ptr;
					input_ptr--;
					hptr = input_ptr;
					// *input_ptr = '*';
					while(input_ptr < work_buffer_ptr){
						*input_ptr = *(input_ptr+1);
						input_ptr++;
					}
					input_ptr = hptr;
					*work_buffer_ptr-- = '\0';
					*work_buffer_ptr = '\0';
					printf("\033[K");	// Erase to end of line
					printf("\r> %s", work_buffer);
					while(mv > 0){
							printf("\033[1D");	// move cursor left
							mv--;
						}
				}
				break;

/* OTHER */ default:
				if(escape == true)
					escape = false;
				else 
				{
					if (work_buffer_ptr < end_buff)	// room to add character ?
					{	
						if(input_ptr == work_buffer_ptr)
						{	// no arrow keys in play

							*work_buffer_ptr++ = c;
							input_ptr = work_buffer_ptr;	       			
							printf("%c", c);

						}
					}
					else
					{		// cursor is not at the end of the input buffer
						move_ptr = work_buffer_ptr++;
						move_ptr++;
						*move_ptr-- = '\0';
						while(move_ptr > input_ptr)
						{
							*move_ptr = *(move_ptr - 1);
							move_ptr--;
						}
						*input_ptr++ = c;
						mv = work_buffer_ptr - input_ptr;
						printf("\r> %s", work_buffer);
						while(mv > 0)
						{
							printf("\033[1D");	// move cursor left
							mv--;
						}
					}
				}			
		}
	/* do suff while waiting or the keyboard */

	}
		
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\n***normal termination -  but should not happen\n\n");
	return 0;
}


int term(int t) {
	// semctl(semid, 0, IPC_RMID, dummy);
	// printf("    semaphore set removed\n\r");
	switch (t) {
	case 1:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("*** normal termination\n\n");
		exit(-1);
		break;
	case 2:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("\f\n*** escape termination\n\n");
		exit(-1);
		break;
	case 3:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("\f\n*** serial error program terminated\n\n");
		exit(-1);
		break;
	default:
		break;
	}
	return 1;
}
void term1(void) {
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	semctl(semid, 0, IPC_RMID, dummy);
	printf("semaphore set removed\n\r");
	printf("\n*** program terminated\n\n");
	exit(-1);
	return;
}
