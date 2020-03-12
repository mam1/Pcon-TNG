


#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include "fifo_q.h"

#define TOKEN_BUFFER_SIZE  50
int main(void)
{
  _TOPIC_Q      *mt = NULL;

  char           buffer[TOKEN_BUFFER_SIZE];
  char           topic[] = "258Thomas/xxxx/zz";

  printf("/ntopic string >%s<\n", topic);

  mt = build_token_q((char*)topic, strlen(topic));
  printf("\n%s\n","print list" );
  list_print(mt);

  memset(buffer, '\0', TOKEN_BUFFER_SIZE);
  while (pop(mt, buffer))
  {
    printf("**** poped token >%s<\n", buffer);
    memset(buffer, '\0', TOKEN_BUFFER_SIZE);
  }
  // if(pop(mt,buffer) == NULL) printf("%s\n", "list is empty\n");

  return 0;
}


