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
#include "ipc.h"

/******************************** globals **************************************/
int				trace_flag;							//control program trace
// int 			exit_flag = false;					//exit man loop if TRUE
int 			bbb;								//UART1 file descriptor
SYS_DAT 		sdat;								//system data structure
CMD_FSM_CB  	cmd_fsm_cb;							//cmd_fsm control block
IPC_DAT 		ipc_dat; 							//ipc data
void			*data = NULL;						//pointer to ipc data
char           	ipc_file[] = {_IPC_FILE};  			//name of ipc file
uint8_t 		cmd_state,char_state;				//fsm current state
char 			work_buffer[_INPUT_BUFFER_SIZE], *work_buffer_ptr;
char 			tbuf[_TOKEN_BUFFER_SIZE];



/***************** global code to text conversion ********************/
char *day_names_long[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *day_names_short[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char *onoff[2] = {"off"," on"};
char *con_mode[3] = {"manual","  time","time & sensor"};
char *sch_mode[2] = {"day","week"};
char *c_mode[4] = {"manual","  time","   t&s"," cycle"};

/***************************** support routines ********************************/
/* write system info to stdout */
void disp_sys(void) {
	printf(" Pcon  %d.%d.%d \n\r", _major_version, _minor_version,_minor_revision);
	printf(" input buffer size: %d characters\n\r", _INPUT_BUFFER_SIZE);
	printf(" system schedule size: %d bytes\r\n",sizeof(cmd_fsm_cb.sdat_ptr->sch));
	printf(" stored schedule templates: %i\r\n",sdat.schlib_index);
										
	return;
}
/* prompt for user input */
void prompt(void){
	printf("%s",cmd_fsm_cb.prompt_buffer);
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


	/*********************** setup console *******************************/
	printf("\033\143"); 				//clear the terminal screen, preserve the scroll back
	printf("*** Pcon  %d.%d.%d ***\n\n\r", _major_version, _minor_version,_minor_revision);
				
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
	printf("ipc data allocated at <%x>\n",(uint32_t)&ipc_dat);
	

    /* set up file mapped shared memory for inter process communication */ 
   	fd = ipc_open(ipc_file, ipc_size());      // create/open ipc file
  	data = ipc_map(fd, ipc_size());           // map file to memory
 // 	memcpy(&ipc_dat, data, sizeof(ipc_dat));  // move shared memory data to local structure

	/* load data from file on sd card */
	load_system_data(_SYSTEM_DATA_FILE,&sdat);
	printf("  Pcon: system data loaded from %s\r\n",_SYSTEM_DATA_FILE);
	if((sdat.major_version!=_major_version ) | (sdat.minor_version!=_minor_version) | (sdat.minor_revision!=_minor_revision)){
		printf("*** versions do not match\r\n");
		printf("  version info from system data file - %d.%d.%d\r\n", sdat.major_version, sdat.minor_version,sdat.minor_revision);
		printf("  version info from program          - %d.%d.%d\r\n", _major_version, _minor_version,_minor_revision);
		printf("  update system data file? <y>|<n>: ");
		if (getchar() == 'y'){
			sdat.major_version = _major_version;
			sdat.minor_version = _minor_version;
			sdat.minor_revision = _minor_revision;
			save_system_data(_SYSTEM_DATA_FILE,&sdat);
			printf("    Pcon: system data file updated\r\n");
		}
		c = fgetc(stdin);	// get rid of trailing CR
	}

	/* copy system data to shared memory */
	memcpy(ipc_dat.sch, sdat.sch, sizeof(sdat.sch));
	for(i=0;i<_NUMBER_OF_CHANNELS;i++){
		ipc_dat.c_dat[i].c_state = sdat.c_data[i].c_state;
		ipc_dat.c_dat[i].c_mode = sdat.c_data[i].c_mode;
		ipc_dat.c_dat[i].on_sec = sdat.c_data[i].on_sec;
		ipc_dat.c_dat[i].off_sec = sdat.c_data[i].off_sec;
	}
	ipc_dat.force_update = 1;			// force daemon to update relays
	memcpy(data, &ipc_dat, sizeof(ipc_dat));  	// move local structure to shared memory
	printf("  Pcon: system data copied to shared memory\r\n");

	/* setup control block pointers */
	cmd_fsm_cb.sdat_ptr = &sdat;	//set up pointer in cmd_fsm control block to allow acces to system data
	cmd_fsm_cb.w_sch_ptr = (uint32_t *)cmd_fsm_cb.w_sch;
	cmd_fsm_cb.sdat_ptr->sch_ptr = (uint32_t *)cmd_fsm_cb.sdat_ptr->sch;

    /* load working schedule from system schedule */
    printf("  Pcon: size of schedule buffer = %i\r\n",sizeof(cmd_fsm_cb.w_sch));
    memcpy(cmd_fsm_cb.w_sch_ptr,cmd_fsm_cb.sdat_ptr->sch_ptr,sizeof(cmd_fsm_cb.w_sch));
    printf("  Pcon: system schedule copied to buffer\r\n");

	/* initialize state machines */
	work_buffer_ptr = (char *)work_buffer;  //initialize work buffer pointer
	cmd_fsm_reset(&cmd_fsm_cb); 	//initialize the command processor fsm
	char_fsm_reset();
	char_state = 0;					//initialize the character fsm

	/* set up unbuffered io */
	fflush(stdout);
	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

#ifdef _TRACE
	trace(_TRACE_FILE_NAME,"Pcon",char_state,NULL,"starting main event loop\n",trace_flag);
#endif
	printf("\r\ninitialization complete\r\n\n");
	disp_sys();	        //display system info on serial terminal
	printf("\r\n\n");
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
			while(pop_cmd_q(cmd_fsm_cb.token)); 	//empty command queue
			cmd_fsm_reset(&cmd_fsm_cb);				//reset command fsm
			for (i = 0; i < _INPUT_BUFFER_SIZE; i++)//clean out work buffer
				work_buffer[i] = '\0';
			char_fsm_reset();						//reset char fsm
			prompted = false;						//force a prompt
			strcpy(cmd_fsm_cb.prompt_buffer,"command processor reset\r\n\nenter a command\r\n> ");
			break;

/* CR */	case _CR:
#ifdef _TRACE
			trace(_TRACE_FILE_NAME,"Pcon",char_state,work_buffer,"character entered is a _CR",trace_flag);
#endif
			fputc(_CR, stdout);						//make the scree look right
			fputc(_NL, stdout);
			*work_buffer_ptr = c;					// load the CR into the work buffer
			work_buffer_ptr = work_buffer;			// reset pointer
			char_fsm_reset();
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
	/* do suff while waiting or the keyboard */	

	};
	// s_close(bbb);
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\f\n***normal termination -  but should not happen\n\n");
	return 0;
}
int term(int t){
	// s_close(bbb);
	switch(t){
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
void term1(void){
	system("/bin/stty cooked");			//switch to buffered iput
	system("/bin/stty echo");			//turn on terminal echo
	printf("\r\n*** program terminated\n\n");
	exit(-1);
	return;
}
