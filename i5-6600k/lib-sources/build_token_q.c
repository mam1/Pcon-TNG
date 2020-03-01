
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "typedefs.h"

#define TOKEN_BUFFER_SIZE  50


_TOPIC_NODE 			*topic_stack_head = NULL;
_TOPIC_NODE				* topic_stack_tail = NULL;
_TOPIC_NODE				*new_node;

void build_token_q(char *topicName, int topicLen)
{
	char			*topicNameptr, *cptr;
	int 			topicNamesize;
	char 			token_buffer[TOKEN_BUFFER_SIZE], *token_buffer_ptr;

	printf("build_token_q called \n");
	printf("%s   %i\n", topicName, topicLen);

	topicNameptr = topicName;
	topicNamesize = topicLen;

	if (*topicNameptr == '/')
	{
		topicNameptr++;
		topicNamesize--;
	}

	memset(token_buffer, '\0', TOKEN_BUFFER_SIZE);
	token_buffer_ptr = (char *)token_buffer;


	while (*topicNameptr != '\0')
	{
		while (*topicNameptr != '/' && *topicNameptr != '\0')
		{
			*token_buffer_ptr++ = *topicNameptr;
			putchar(*topicNameptr++);
		}
		putchar('\n');

		// allocate allocate memory for token
		printf("buffer size %i\n", (int)strlen(token_buffer));

		cptr = (char *)malloc((strlen(token_buffer)+1) * strlen(token_buffer));
		if( NULL == cptr )
	    {
	      fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "token alocate");
	      return; 
	    }

		strncpy(cptr, token_buffer, strlen(token_buffer));
		printf("token  <%s>\n", (char *)token_buffer);
		printf("token2  <%s>\n", cptr);

		

		printf("size of node %i\n", (int)sizeof(*new_node));
		new_node = (_TOPIC_NODE *)malloc(sizeof(*new_node));
		if( NULL == new_node )
	    {
	      fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "node alocate");
	      return; 
	    }
	    new_node->token = cptr;






		memset(token_buffer, '\0', TOKEN_BUFFER_SIZE);
		token_buffer_ptr = (char *)token_buffer;
		topicNameptr++;
	}

}
