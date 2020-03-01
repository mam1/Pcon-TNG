 


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

  char          *t1 = "258Thomas";
  char          *t2 = "test";
  char          *t3 = "the";
  char          *t4 = "q";
 
  mt = list_new();

  list_add_element(mt, t1);
  list_add_element(mt, t2);
  list_add_element(mt, t3);
  list_add_element(mt, t4); 
   
  list_print(mt);
  memset(buffer, '\0', TOKEN_BUFFER_SIZE);
  pop (mt,buffer);

  // buffer[0] = 'h';
  // buffer[1] = 'i';
  // buffer[2] = '\0';
  printf("buffer <%s>\n", buffer);

  // n=list_remove_element(mt);

 // printf("removed token <%s>\n", n->head->token);

 //  list_print(mt);
 
  // list_free(mt);   /* always remember to free() the malloc()ed memory */
  // free(mt);        /* free() if list is kept separate from free()ing the structure, I think its a good design */
  // mt = NULL;      /* after free() always set that pointer to NULL, C will run havon on you if you try to use a dangling pointer */
 
  // list_print(mt);
 
  return 0;
}
 
 
 