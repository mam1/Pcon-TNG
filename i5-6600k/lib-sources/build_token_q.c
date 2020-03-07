
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "typedefs.h"
#include "fifo_q.h"

#define TOKEN_BUFFER_SIZE  50

_TOPIC_Q * build_token_q(char *topicName, int topicLen)
{
	char			*topicNameptr, *cptr;
	// int 			topicNamesize;
	char 			token_buffer[TOKEN_BUFFER_SIZE], *token_buffer_ptr;
	_TOPIC_Q      	*mt = NULL;

	printf("build_token_q called \n");

	mt = list_new();
	topicNameptr = topicName;

	memset(token_buffer, '\0', TOKEN_BUFFER_SIZE);
	token_buffer_ptr = (char *)token_buffer;

	while (*topicNameptr != '\0') 
	{
		while (*topicNameptr != '\0' && *topicNameptr != '/')
			*token_buffer_ptr++ = *topicNameptr++;
		if(*topicNameptr == '/') topicNameptr++;
		printf("allocating memory for token buffer size %i\n", (int)strlen(token_buffer));
		cptr = (char *)malloc((strlen(token_buffer) + 1) * sizeof(char));
		if ( NULL == cptr )
		{
			fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "token alocate");
			return NULL;
		}
		strncpy(cptr, token_buffer, strlen(token_buffer));
		list_add_element(mt, cptr);
		memset(token_buffer, '\0', TOKEN_BUFFER_SIZE);
		token_buffer_ptr = (char *)token_buffer;
	}

	return mt;
}
