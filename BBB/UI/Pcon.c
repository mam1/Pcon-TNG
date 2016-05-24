/**
 * This is the main Pcon.c program file
 * BeagleBone Black implementation
 */

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
int 			bbb;								//UART1 file descriptor
_CMD_FSM_CB  	cmd_fsm_cb;							//cmd_fsm control block
_IPC_DAT 		*ipc_ptr; 							//ipc data
void			*data = NULL;						//pointer to ipc data
char           	ipc_file[] = {_IPC_FILE_NAME};  			//name of ipc file
uint8_t 		cmd_state, char_state;				//fsm current state
char 			work_buffer[_INPUT_BUFFER_SIZE], *work_buffer_ptr;
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

// _SYS_DAT 		sys_data;								//system data


/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *day_names_short[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *onoff[2] = {"off", " on"};
char *con_mode[3] = {"manual", "  time", "time & sensor"};
char *sch_mode[2] = {"day", "week"};
char *mode[4] = {"manual", "  time", "sensor", " cycle"};

/***************************** support routines ********************************/

/* check system status */



/* prompt for user input */
void prompt(int s) {

	// printf("************** prompt called\r\n");
	// printf("********** record count before call to build_prompt %i\r\n", cmd_fsm_cb.w_template_buffer.rcnt);	
	build_prompt(&cmd_fsm_cb);
	printf("%s <%i> ", cmd_fsm_cb.prompt_buffer,s);
	// printf("************** prompt returning\r\n");	
	return;
}

/* load buffer with previous command */
void up_arrow(void){

	printf("up arrow detected\n\r");
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
	FILE 			*sys_file, f;
	_CONFIG_DAT 	hold_config;
	// char 			*command_buffer[];	//

	/*********************** setup console *******************************/
	printf("\033\143"); 				//clear the terminal screen, preserve the scroll back
	printf("*** Pcon  %d.%d.%d ***\n\n\r", _MAJOR_VERSION, _MINOR_VERSION, _MINOR_REVISION);

	/************************* setup trace *******************************/
#ifdef _TRACE
	trace_flag = true;
#else
	trace_flag = false;
#endif
	if (trace_flag == true) {
		printf(" program trace active,");
		if (trace_on(_TRACE_FILE_NAME, &trace_flag)) {
			printf("trace_on returned error\n");
			trace_flag = false;
		}
	}
	if (trace_flag == false)
		printf(" program trace disabled\n");

	/************************ initializations ****************************/
#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", char_state, NULL, "starting initializations", trace_flag);
#endif
	/* set up file mapped shared memory for inter process communication */
	ipc_sem_init();								// setup semaphores
#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", char_state, NULL, "semaphores initialized", trace_flag);
#endif	
	semid = ipc_sem_id(skey);					// set semaphor id		
#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", char_state, NULL, "semaphores id set", trace_flag);
#endif	

	/* set up shared memory */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	fd = ipc_open(ipc_file, ipc_size());      	// create/open ipc file
	data = ipc_map(fd, ipc_size());           	// map file to memory
	ipc_ptr = data; 							// overlay data with _IPC_DAT data structure
	ipc_sem_free(semid, &sb);					// free lock on shared memory

	/* setup control block pointers */
	cmd_fsm_cb.ipc_ptr = ipc_ptr;					 	//set pointer to shared memory
	cmd_fsm_cb.sys_ptr = &ipc_ptr->sys_data;		 	//set pointer to system data in shared memory
	cmd_fsm_cb.ssch_ptr = &ipc_ptr->sys_data.sys_sch; 	//set pointer to active shecule in shared memory
	cmd_fsm_cb.wsch_ptr = &cmd_fsm_cb.w_sch;		 	//set pointer to working schedule

	// cmd_fsm_cb.w_sch_ptr = &cmd_fsm_cb.w_sch;		 //set pointer to working schedule
	// cmd_fsm_cb.sch_ptr = &ipc_ptr->sys_data.sys_sch; //set pointer to active shecule in shared memory
	// cmd_fsm_cb.cdat_ptr = &ipc_ptr->sys_data.c_data; //set pointer to channel data array in shared memory

    /* load data from system data file and compare config data */
    sys_file = sys_open(_SYSTEM_FILE_NAME,&ipc_ptr->sys_data);  // create system file if it does not exist
    // sys_load(sys_file,&ipc_ptr->sys_data);
    sys_load(sys_file,cmd_fsm_cb.sys_ptr);
    fclose(sys_file);

    hold_config = cmd_fsm_cb.sys_ptr->config;
    // printf("loaded minor_revision from system file %i\n",hold_config.minor_revision);
    if(sys_comp(&hold_config)){
    	printf("*** there are different configurations in the system file and in the application\n update system file? (y)|(n) > ");
		if (getchar() == 'y') {
		    cmd_fsm_cb.sys_ptr->config.major_version = _MAJOR_VERSION;
		    cmd_fsm_cb.sys_ptr->config.minor_version = _MINOR_VERSION;
		    cmd_fsm_cb.sys_ptr->config.minor_revision = _MINOR_REVISION;
		    cmd_fsm_cb.sys_ptr->config.channels = _NUMBER_OF_CHANNELS;
		    cmd_fsm_cb.sys_ptr->config.sensors = _NUMBER_OF_SENSORS;
		    cmd_fsm_cb.sys_ptr->config.commands = _CMD_TOKENS;
		    cmd_fsm_cb.sys_ptr->config.states = _CMD_STATES;

		    sys_file = sys_open(_SYSTEM_FILE_NAME, cmd_fsm_cb.sys_ptr);
			if(sys_save(sys_file,cmd_fsm_cb.sys_ptr)){
    			printf("\n *\n*** unable to save system data to file <%s>\n", _SYSTEM_FILE_NAME);
				exit(1);
			}
			else
				printf("  system data file updated\r\n");

			c = fgetc(stdin);			// get rid of trailing CR
			fclose(sys_file);
		}
		else {
			c = fgetc(stdin);			// get rid of trailing CR
	    	printf("*** application terminated\n");
	    	exit(1);
	    }
    }
    // fclose(sys_file);

    /* test to make sure save is working */
    sys_file = sys_open(_SYSTEM_FILE_NAME,cmd_fsm_cb.sys_ptr);
    if(sys_save(sys_file,&ipc_ptr->sys_data)){
        printf("\n *\n*** unable to save system data to file <%s>\n", _SYSTEM_FILE_NAME);
        exit(1);	
    }
    fclose(sys_file);
    
#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", char_state, NULL, "system data loaded into shared memory", trace_flag);
	printf("  Pcon: system data loaded into shared memory\r\n");
#endif

	/* load working schedule from system schedule */
	ipc_sem_lock(semid, &sb);					// wait for a lock on shared memory
	cmd_fsm_cb.w_sch = ipc_ptr->sys_data.sys_sch; 
	ipc_sem_free(semid, &sb);					// free lock on shared memory

#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", char_state, NULL, "system schedule copied to working schedule", trace_flag);
	printf("  Pcon: system schedule copied to working schedule\r\n");
#endif

	/* initialize state machines */
	work_buffer_ptr = (char *)work_buffer;  //initialize work buffer pointer
	cmd_fsm_reset(&cmd_fsm_cb); 			//initialize the command processor fsm
	char_fsm_reset();
	char_state = 0;							//initialize the character fsm

	/* set up unbuffered io */
	fflush(stdout);
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

#ifdef _TRACE
	trace(_TRACE_FILE_NAME, "\nPcon", 0, NULL, "initializations complete\n", trace_flag);
	trace(_TRACE_FILE_NAME, "\nPcon", 0, NULL, "starting main event loop\n", trace_flag);
#endif
	printf(" initializations complete\r\n\n");

	/* set initial prompt */
	strcpy(cmd_fsm_cb.prompt_buffer, " enter a command");

	/************************************************************/
	/**************** start main processing loop ****************/
	/************************************************************/

	while (1) {
		/* check the token stack */
		while (pop_cmd_q(cmd_fsm_cb.token))
		{
			// printf("************ before cycle\r\n");
			// printf("********** record count before cycle %i\r\n", cmd_fsm_cb.w_template_buffer.rcnt);
			cmd_fsm(&cmd_fsm_cb);   	//cycle cmd fsm until queue is empty
			// printf("************ after cycle\r\n");
			// printf("********** record count after cycle %i\r\n", cmd_fsm_cb.w_template_buffer.rcnt);
			prompted = false;
		}
		if (prompted == false) {				//display prompt if necessary
			// printf("************* before prompt\r\n");
			// printf("********** record count before call prompt %i\r\n", cmd_fsm_cb.w_template_buffer.rcnt);
			prompted = true;
			prompt(cmd_fsm_cb.state);
		}
		
		c = fgetc(stdin);
		switch (c) {
	/* NOCR */ case _NO_CHAR:
			break;
	/* ESC */  case _ESC:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "escape entered", trace_flag);
#endif
			/* detect up arrow */
			c = fgetc(stdin);		// skip next character
			c = fgetc(stdin);
			if(c == 'A') {
				up_arrow();
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "up arrow entered", trace_flag);
#endif
				break;
			}

			/* escape entered */
			while (pop_cmd_q(cmd_fsm_cb.token)); 	//empty command queue
			cmd_fsm_reset(&cmd_fsm_cb);				//reset command fsm
			for (i = 0; i < _INPUT_BUFFER_SIZE; i++)//clean out work buffer
				work_buffer[i] = '\0';
			char_fsm_reset();						//reset char fsm
			prompted = false;						//force a prompt
			strcpy(cmd_fsm_cb.prompt_buffer, "\r\ncommand processor reset\n\renter a command");
			break;

	/* CR */	case _CR:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "character entered is a _CR", trace_flag);
#endif
			fputc(_CR, stdout);						//make the scree look right
			fputc(_NL, stdout);
			*work_buffer_ptr = c;					// load the CR into the work buffer
			work_buffer_ptr = work_buffer;			// reset pointer
			char_fsm_reset();
			while (*work_buffer_ptr != '\0')			//send the work buffer content to the fsm
				char_fsm(char_type(*work_buffer_ptr), &char_state, work_buffer_ptr++); //cycle fsm
			for (i = 0; i < _INPUT_BUFFER_SIZE; i++)		//clean out work buffer
				work_buffer[i] = '\0';
			work_buffer_ptr = work_buffer;			//reset pointer
			break;
	/* DEL */   case _DEL:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "character entered is a _BS", trace_flag);
#endif
			fputc(_BS, stdout);
			fputc(' ', stdout);
			fputc(_BS, stdout);
			*work_buffer_ptr-- = '\0';
			*work_buffer_ptr = '\0';
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "remove character from input buffer", trace_flag);
#endif
			break;		

	/* OTHER */ default:
			fputc(c, stdout);       				// echo char
			*work_buffer_ptr++ = c;
#ifdef _TRACE
			trace(_TRACE_FILE_NAME, "\nPcon", char_state, work_buffer, "add character to work buffer", trace_flag);
#endif
		}
		/* do suff while waiting or the keyboard */

	};
	// s_close(bbb);
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
