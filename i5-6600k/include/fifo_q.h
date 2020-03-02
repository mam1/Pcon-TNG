
/**************************************************/
#ifndef FIFO_Q_H_
#define FIFO_Q_H_


/*  version info */
#define _MAJOR_VERSION_FIFO_Q   	0
#define _MINOR_VERSION_FIFO_Q   	0
#define _MINOR_REVISION_FIFO_Q  	0

#include "typedefs.h"

/* fuctions */

_TOPIC_Q* list_add_element(_TOPIC_Q*, char *);
_TOPIC_Q* list_remove_element( _TOPIC_Q*);
_TOPIC_Q* list_free( _TOPIC_Q*);
_TOPIC_Q* list_new(void);

void list_print_element(_TOPIC_NODE *);
void list_print(_TOPIC_Q *);
void pop(_TOPIC_Q *, char *);

_TOPIC_Q* build_token_q(char *, int);


#endif
