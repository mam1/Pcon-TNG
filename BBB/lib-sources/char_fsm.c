
/* char_fsm version info */
#define _MAJOR_VERSION    10
#define _MINOR_VERSION    4
#define _MINOR_REVISION   0


#include <stdio.h>
#include <string.h> 	//memset, memcpy
#include <ctype.h> 		//isalnum, tolower
#include <stdint.h>		//uint_8, uint_16, uint_32, etc.
#include <stdlib.h>
// #include <malloc.h>
#include "Pcon.h"
#include "typedefs.h"
#include "char_fsm.h"
#include "cmd_fsm.h"
#include "trace.h"

/***************************** globals ******************************/
char input_buffer[_INPUT_BUFFER_SIZE], *input_buffer_ptr;

/***************************** globals ******************************/
TQ *head, *tail;
#if defined (_ATRACE) || defined (_FTRACE)
char			trace_buf[_TRACE_BUFFER_SIZE];
#endif
/***************************** externals ************************/
extern int			trace_flag;
extern int cmd_state, char_state;
extern struct {
	int channel;
	int day;
	int hour;
	int minute;
	int key;
	// uint32_t edit_buffer[_MAX_SCHEDULE_RECS];
	// uint32_t clipboard_buffer[_MAX_SCHEDULE_RECS];

} edit;
/********************** support functions ****************************/
TQ *process_buffer(void) {
	#if defined (_ATRACE) || defined (_FTRACE)
		trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "called", trace_flag);
	#endif
	char 			tb[_INPUT_BUFFER_SIZE], *t_ptr, *start_char;        //
	int 			i;

	input_buffer_ptr = input_buffer;	// set pointer to start of input buffer
	start_char = input_buffer_ptr;		// set start_char to start of input buffer
	t_ptr = tb;							// set pointer to start of temporary buffer
	memset(&tb[0], 0, sizeof(tb));		// initialize temporary buffer

	head = '\0';						// initialize head pointer
	tail = head;						// initialize tail pointer


	while (*input_buffer_ptr != '\0') { // loop until the input buffer is empty
		// #if defined (_ATRACE) || defined (_FTRACE)
		// trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "carrage return", trace_flag);
		// #endif
		
		/* NULL */
			if(input_buffer_ptr == input_buffer){

			#if defined (_ATRACE) || defined (_FTRACE)
				trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "null buffer", trace_flag);
			#endif
			}

		/* QUOTE */ 
			if (*input_buffer_ptr == _QUOTE) {
				#if defined (_ATRACE) || defined (_FTRACE)
					trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "found a quote", trace_flag);
				#endif
				*t_ptr++ = *input_buffer_ptr++;
				while ((*input_buffer_ptr != _QUOTE) && (*input_buffer_ptr != '\0')) // look for ending quote or end of buffer
					*t_ptr++ = *input_buffer_ptr++;									 // move char to temp buffer
				*t_ptr++ = _QUOTE;													 // add ending quite to temp buffer
				#if defined (_ATRACE) || defined (_FTRACE)
					trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "found a second quote", trace_flag);
				#endif
				*(++input_buffer_ptr) = '\0';
				if (tail == '\0') {
					tail = malloc(sizeof(TQ));
					head = tail;
				}
				else {
					tail->next = malloc(sizeof(TQ));
					tail = tail->next;
				}
				tail->tptr = malloc(input_buffer_ptr - start_char);
				memcpy(tail->tptr, start_char, input_buffer_ptr - start_char + 1);
				tail->next = '\0';
				start_char = input_buffer_ptr;
				start_char++;
			#if defined (_ATRACE) || defined (_FTRACE)
				sprintf(trace_buf, "pusn token <%s>", tail->tptr);
				trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, trace_buf, trace_flag);
			#endif
		}

		/* DELIM */ 
			if (char_type(*input_buffer_ptr) == 0) {					//test for a delimiter
			#if defined (_ATRACE) || defined (_FTRACE)
				trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "found a delimiter", trace_flag);
				trace(_TRACE_FILE_NAME, "temp buffer", char_state, tb, "character added to temp buffer", trace_flag);
			#endif
			*input_buffer_ptr = '\0';
			if (tail == '\0') {
				tail = malloc(sizeof(TQ));
				head = tail;
			}
			else {
				tail->next = malloc(sizeof(TQ));
				tail = tail->next;
			}
			
			tail->tptr = malloc(input_buffer_ptr - start_char + 1);
			// printf("****************** %i\n\r", input_buffer_ptr - start_char);
			memcpy(tail->tptr, start_char, input_buffer_ptr - start_char + 1);
			tail->next = '\0';
			start_char = input_buffer_ptr;
			start_char++;
			#if defined (_ATRACE) || defined (_FTRACE)
				sprintf(trace_buf, "push token <%s>", tail->tptr);
				trace(_TRACE_FILE_NAME, "process_buffer", char_state, tb, trace_buf, trace_flag);
				trace(_TRACE_FILE_NAME, "temp buffer", char_state, tb, "character added to temp buffer", trace_flag);
			#endif
		}

		*t_ptr++ = *input_buffer_ptr++;

// printf("*** input buffer offset %i <%s>\n\r", (int)(input_buffer_ptr - input_buffer), input_buffer);
// printf("***  temp buffer offset %i <%s>\n\r\n", (int)(t_ptr - tb),tb);

		#if defined (_ATRACE) || defined (_FTRACE)
			trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "character added to temp buffer", trace_flag); 
			trace(_TRACE_FILE_NAME, "temp buffer", char_state, tb, "character added to temp buffer", trace_flag);
		#endif
	}



	for (i = 0; i < _INPUT_BUFFER_SIZE; i++)		//clean out input buffer
		input_buffer[i] = '\0';
	input_buffer_ptr = input_buffer;				//reset pointer
	#if defined (_ATRACE) || defined (_FTRACE)
		trace(_TRACE_FILE_NAME, "process_buffer", char_state, input_buffer, "done processing, clean up", trace_flag);
	#endif

	return head;
}

/* return token code  */
int char_type(char c) {
	switch (c) {
	case _COMMA:
	case _SPACE:
	case _COLON:
	case _SLASH:
		return 0;
	case _QUOTE:
		return 1;
	case _DEL:
		return 2;
	case _CR:
		return 3;
	}
	return 4;
}


int char_fsm_reset(void) {
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "char_fsm_reset", char_state, input_buffer, "resetting", trace_flag);
#endif
	char_state = 0;
	input_buffer_ptr = input_buffer;
	memset(input_buffer, '\0', sizeof(input_buffer));
	input_buffer_ptr = input_buffer;
	return 0;
}

int reset_cmd_fsm(void) {
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "reset_cmd_fsm", char_state, input_buffer, "resetting", trace_flag);
#endif
	abort();
	char dump[_TOKEN_BUFFER_SIZE];
	cmd_state = 0;          //reset state
	while (pop_cmd_q(dump))
		; //clear out the command queue

	printf("\n\nfsm reset\n");
//    reset_edit();           // clean out edit buffers
	input_buffer_ptr = input_buffer;
	*input_buffer_ptr++ = '?';
	*input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = '\0';

	process_buffer();
	return 0;
}

/********************************************/
/** character input parser action routines **/
/********************************************/
/* do nothing */
int nop(char *c) {
	if ((*c == _CR) && (char_state == 0)) {
		fputc(_CR, stdout);
		fputc('>', stdout);
		fputc(' ', stdout);
	}
	return 0;
}
/* add char to buffer */
int add(char *c) {
	*input_buffer_ptr++ = *c;
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "add", char_state, input_buffer, "adding character to buffer", trace_flag);
#endif
	return 0;
}
/* delim + quote to buffer */
int adq(char *c) {
	*input_buffer_ptr++ = ' ';
	*input_buffer_ptr++ = *c;

#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "add", char_state, input_buffer, "adding character to buffer", trace_flag);

#endif
	return 0;
}
/* add quote + delim to buffer */
int aqd(char *c) {
	*input_buffer_ptr++ = *c;
	*input_buffer_ptr++ = ' ';

#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "add", char_state, input_buffer, "adding character to buffer", trace_flag);
#endif
	return 0;
}
/* remove char from buffer */
int del(char *c) {
	input_buffer_ptr--;
	*input_buffer_ptr = '\0';
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "del", char_state, input_buffer, "removing character from buffer", trace_flag);
#endif
	return 0;
}

/*  add delimitier to buffer */
int dlm(char *c) {
	*input_buffer_ptr++ = *c;
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "dlm", char_state, input_buffer, "add delimiter to buffer", trace_flag);
#endif
	return 0;
}
/* process buffer */
int cr(char *c) {
	if (char_type(*input_buffer_ptr) != 0) *input_buffer_ptr++ = ' ';	//make sure the buffer ends with white space
	*input_buffer_ptr++ = '\0';
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "cr", char_state, input_buffer, "process buffer", trace_flag);
#endif										//with a blank folowed by a NULL
	process_buffer();
	return 0;
}

int cr2(char *c) {
	*input_buffer_ptr = '\0';

#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "cr2", char_state, input_buffer, "process buffer", trace_flag);
#endif										//with a blank folowed by a NULL
	process_buffer();
#if defined (_ATRACE) || defined (_FTRACE)
	system ("/bin/stty cooked");			//switch to buffered iput
	system("stty echo");					//turn on terminal echo

	system("stty -echo");					//turn off terminal echo
	system("/bin/stty raw");				// use system call to make terminal send all keystrokes directly to stdin
#endif

	return 0;
}
/* 5 -  send a empty buffer  */
int snul(char *c) {
#if defined (_ATRACE) || defined (_FTRACE)
	trace(_TRACE_FILE_NAME, "snul", char_state, input_buffer, "add quote, process buffer", trace_flag);
#endif
// 	*input_buffer_ptr++ = _QUOTE;
// 	*input_buffer_ptr++ = ' ';
// 	*input_buffer_ptr++ = '\0';
// 	process_buffer();
	return 0;
}

/* 7 – add delimiter to buffer, add quote to buffer */
int char_delim_add(char *c) {
	*input_buffer_ptr++ = _QUOTE;
	return 0;
}

/* 8  - add char to buffer,  process buffer */
int char_eof_process(char *c) {
	*input_buffer_ptr++ = '\0';
	process_buffer();
	return 0;
}

/***************************************/
/**  character input parser fsm start **/
/***************************************/

/* fsm support functions */
int char_type(char);
TQ *process_buffer(void);

/* fsm fuctions */
int nop(char *);	//do nothing
int del(char *);	//remove character from input buffer
int add(char *);	//add character to input buffer
int aqd(char *);	//add quote + delim to input buffer
int adq(char *);	//delim  + quote to input buffer
//int dlm(char *);	//add delimiter to buffer
int cr(char *);		//process input buffer, reset char_fsm
int cr2(char *);	//remove trailing delimiter, process buffer, reset char_fsm
int snul(char *);

/* character processor action table - initialized with fsm functions */

CHAR_ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {
	/* DELIM */{nop,  add,  add,  nop},
	/* QUOTE */{add,  aqd,  adq,  add},
	/*   DEL */{del,  del,  del,  del},
	/*    CR */{cr,  cr,   cr,   cr2},
	/* OTHER */{add,  add,  add,  add}
};

/* character processor state transition table */
int char_new_state[_CHAR_TOKENS][_CHAR_STATES] = {

	/* DELIM */{ 0, 1, 3, 3},
	/* QUOTE */{ 1, 3, 1, 1},
	/*   DEL */{ 0, 1, 2, 3},
	/*    CR */{ 0, 0, 0, 0},
	/* OTHER */{ 2, 1, 2, 2}
};

/*****************************************************/
/****  character input parser state machine end  *****/
/*****************************************************/

void char_fsm(int c_type, int *state, char *c) 
{
	char_action[c_type][*state](c);
	*state = char_new_state[c_type][*state];
	return;
}

/* test command queue return: 0 empty queue, -1 toke available */
int test_cmd_q(void) 
{
	if (head == '\0')
		return 0;
	return -1;
}

/* pop token into buffer, return: 0 empty queue, -1 data placed in buffer  */
int pop_cmd_q(char *buf) {
// #if defined (_ATRACE) || defined (_FTRACE)
// 	// trace1(_TRACE_FILE_NAME,"pop_cmd_q: called",1);
// #endif
	TQ *hold;
	char *ptr1, *ptr2;

	if (head == '\0') {
		*buf = '\0';    //set buffer to empty
		return 0;
	}
	ptr1 = head->tptr;
	ptr2 = buf;
	hold = head;
	while (*ptr1 != '\0')
		*ptr2++ = *ptr1++;
	*ptr2 = '\0';
	head = head->next;
	free(hold->tptr);
	free(hold);

	return -1;
}

