
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "typedefs.h"

#define TOKEN_BUFFER_SIZE  50


_TOPIC_NODE 			*topic_stack_head = NULL;
_TOPIC_NODE				* topic_stack_tail = NULL;

void build_token_q(char *topicName, int topicLen)
{
	int 			i;
	char			*topicNameptr, *topicNameEnd;
	int 			topicNamesize;
	char 			token_buffer[TOKEN_BUFFER_SIZE], *token_buffer_ptr;

	printf("build_token_q called \n");
	printf("%s   %i\n", topicName, topicLen);

	topicNameptr = topicName;
	topicNamesize = topicLen;
	topicNameEnd = topicLen + topicNameptr;

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
		// push token on to queue
		// allocate a node
		




		topicNameptr++;
	}

}
