/**
 * This is the main Pcon.c program file
 * BeagleBone Black implementation
 */
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
#include "serial_io.h"

// #include "cmd_fsm.h"

/******************************** globals **************************************/
int				trace_flag;							//control program trace
int 			exit_flag = false;					//exit man loop if TRUE
int 			bbb;								//UART1 file descriptor
SYS_DAT 		sdat;								//system data structure

CMD_FSM_CB  	cmd_fsm_cb;	//cmd_fsm control block


char 			work_buffer[_INPUT_BUFFER_SIZE], *work_buffer_ptr;
char 			tbuf[_TOKEN_BUFFER_SIZE];

uint8_t cmd_state,char_state;
/***************** global code to text conversion ********************/
char *day_names_long[7] = {
     "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char *onoff[2] = {"off"," on"};
char *con_mode[3] = {"manual","  time","time & sensor"};
char *sch_mode[2] = {"day","week"};

/***************************** support routines ********************************/
/* write system info to stdout */
void disp_sys(void) {
	printf("*** Pcon  %d.%d.%d ***\n\n", _major_version, _minor_version,
	_minor_revision);
	printf(" input buffer size: %d characters\n", _INPUT_BUFFER_SIZE);
	return;
}
/* prompt for user input */
void prompt(void){
	printf("%s",cmd_fsm_cb.prompt_buffer);
}

/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
int main(void) {
	uint8_t c;       			//character typed on keyboard
	int	char_state;			//current state of the character processing fsm
	int prompted = false;	//has a prompt been sent
	int i;
	// FILE *sd_card;	
	/************************* setup trace *******************************/
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active,");
		if(trace_on(_TRACE_FILE_NAME,&trace_flag)){
			printf("trace_on returned error\n");
			trace_flag = false;
		}
	}
	if (trace_flag == false)
		printf(" program trace disabled\n");

	/************************ initializations ****************************/
	printf("\033\143"); //clear the terminal screen, preserve the scroll back
	disp_sys();	        //display system info on serial terminal

	/* set up unbuffered io */
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

	/* open UART1 to connect to BBB */
	bbb = s_open();
	printf(" serial device opened handle = %d\r\n",bbb);

	/* load data from file on sd card */
	load_channel_data(_SYSTEM_DATA_FILE,&sdat);
	printf(" system data loaded\r\n");
	printf(" version info from system data file - %d.%d.%d\r\n", sdat.major_version, sdat.minor_version,
	sdat.minor_revision);

	work_buffer_ptr = work_buffer;    	//initialize work buffer pointer
	char_state = 0;						//initialize the character fsm
	cmd_state = 0;                     	//initialize the command processor fsm
	exit_flag = 1;

#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,NULL,"starting main event loop\n",trace_flag);
#endif
	printf("initialization complete\r\n\n");
	/* set initial prompt */
	strcpy(cmd_fsm_cb.prompt_buffer,"enter a command\r\n> ");
	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/
	while (1){
        /* check the token stack */
        while(pop_cmd_q(cmd_fsm_cb.token))
        {
            cmd_fsm(&cmd_fsm_cb);   	//cycle cmd fsm until queue is empty
            prompted = false;
		}	
		if(prompted == false){				//display prompt if necessary
			prompted = true;
			prompt();
		}
		c = fgetc(stdin);
		switch (c) {
/* NOCR */ case _NO_CHAR:
			   break;
/* ESC */  case _ESC:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"escape entered",trace_flag);
#endif
			exit_flag = 0;
			// system("/bin/stty cooked");		//switch to buffered input
			// system("stty echo");				//turn on terminal echo
			while(pop_cmd_q(cmd_fsm_cb.token)); //empty command queue
			cmd_fsm_reset(&cmd_fsm_cb);			//reset command fsm
			// char_fsm_reset();
			prompted = false;
			printf("\nsystem reset\n");
			break;

/* CR */	case _CR:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"character entered is a _CR",trace_flag);
#endif
			fputc(_CR, stdout);						//make the scree look right
			fputc(_NL, stdout);
			*work_buffer_ptr = c;					// load the CR into the work buffer
			work_buffer_ptr = work_buffer;
			reset_char_fsm();
			while(*work_buffer_ptr != '\0')			//send the work buffer content to the fsm
				char_fsm(char_type(*work_buffer_ptr),&char_state,work_buffer_ptr++);  //cycle fsm
			for (i = 0; i < _INPUT_BUFFER_SIZE; i++)		//clean out work buffer
				work_buffer[i] = '\0';
			work_buffer_ptr = work_buffer;			//reset pointer
			break;
/* DEL */   case _DEL:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"character entered is a _BS",trace_flag);
#endif
			fputc(_BS,stdout);
			fputc(' ',stdout);
			fputc(_BS,stdout);
			*work_buffer_ptr-- = '\0';
			*work_buffer_ptr = '\0';
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"remove character from input buffer",trace_flag);
#endif
			break;

/* OTHER */ default:
			fputc(c, stdout);       				// echo char
			*work_buffer_ptr++ = c;
#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"add character to work buffer",trace_flag);
#endif
		}

	};
	s_close(bbb);
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\nnormal termination\n\n");
	return 0;
}
int term(int t){
	s_close(bbb);
	switch(t){
	case 1:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("\f\nnormal termination\n\n");
		exit(-1);
		break;
	case 2:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("\f\nescape termination\n\n");
		exit(-1);
		break;
	case 3:
		system("/bin/stty cooked");			//switch to buffered iput
		system("/bin/stty echo");			//turn on terminal echo
		printf("\f\nserall error program terminated\n\n");
		exit(-1);
		break;
	default:
			break;
	}
	return 1;
}
void term1(void){
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\r\n*** program terminated\n\n");
	exit(-1);
	return;
}
