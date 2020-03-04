 


#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include "fifo_q.h"
 
#define TOKEN_BUFFER_SIZE  50
int main(void)
{
  _TOPIC_Q      *mt = NULL;
  _TOPIC_Q      *n;
  char           buffer[TOKEN_BUFFER_SIZE];

  mt = build_token_q("258Thomas/xxxx", 14);
   
  list_print(mt);
  memset(buffer, '\0', TOKEN_BUFFER_SIZE);
  // pop (mt,buffer);
  printf("poped token >%s<\n", buffer);
  // list_print(mt);


  return 0;
}
 
 
 