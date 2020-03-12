#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include "typedefs.h"
#include "fifo_q.h"

/* Will always return the pointer to topic_q */
_TOPIC_Q * list_add_element(_TOPIC_Q* s, char *tokenptr)
{

  _TOPIC_NODE           *new_node;

  new_node = (_TOPIC_NODE *)malloc(sizeof(*new_node));
  if ( NULL == new_node )
  {
    fprintf(stderr, "IN %s, %s: malloc() failed\n", __FILE__, "node alocate");
    return s;
  }
  new_node->token = tokenptr;
  new_node->next = NULL;

  if ( NULL == s )
  {
    printf("Queue not initialized\n");
    free(new_node);
    return s;
  }
  else if ( NULL == s->head && NULL == s->tail )
  {
    /* printf("Empty list, adding p->num: %d\n\n", p->num);  */
    s->head = s->tail = new_node;
    return s;
  }
  else if ( NULL == s->head || NULL == s->tail )
  {
    fprintf(stderr, "There is something seriously wrong with your assignment of head/tail to the list\n");
    free(new_node);
    return NULL;
  }
  else
  {
    /* printf("List not empty, adding element to tail\n"); */
    s->tail->next = new_node;
    s->tail = new_node;
  }
  return s;
}

/* load the token from the first node into a buffer and remove the node */
_TOPIC_Q*  pop(_TOPIC_Q * topic_q, char * buffer)
{
  _TOPIC_Q                      *rtn;

  if (topic_q->head == NULL || topic_q == NULL)                                            // if the list is empty return NULL
    return NULL;

  strncpy(buffer, topic_q->head->token, strlen(topic_q->head->token));  // save the token
  rtn = list_remove_element(topic_q);                                   // remove the elememnt from the list

  return rtn;                                


}

/* This is a queue and it is FIFO, so we will always remove the first element */
_TOPIC_Q* list_remove_element( _TOPIC_Q* s )
{
  _TOPIC_NODE* h = NULL;
  _TOPIC_NODE* p = NULL;

  if ( NULL == s )
  {
    printf("List is empty\n");
    return NULL;
  }
  else if ( NULL == s->head && NULL == s->tail )
  {
    printf("Well, List is empty\n");
    return NULL;
  }
  else if ( NULL == s->head || NULL == s->tail )
  {
    printf("There is something seriously wrong with your list\n");
    printf("One of the head/tail is empty while other is not \n");
    return NULL;
  }

  h = s->head;
  p = h->next;
  free(s->head->token);
  free(s->head);
  s->head = p;
  if ( NULL == s->head )  s->tail = s->head;  /* The element tail was pointing to is free(), so we need an update */

  return s;
}

/* ---------------------- small helper fucntions ---------------------------------- */
_TOPIC_Q* list_free( _TOPIC_Q* s )
{
  while ( s->head )
  {
    list_remove_element(s);
  }

  return s;
}

_TOPIC_Q* list_new(void)
{
  _TOPIC_Q* p = malloc( 1 * sizeof(*p));

  if ( NULL == p )
  {
    fprintf(stderr, "LINE: %d, malloc() failed\n", __LINE__);
  }

  p->head = p->tail = NULL;

  return p;
}

void list_print_element(_TOPIC_NODE *p )
{
  if ( p )
  {
    printf("token = %s\n", p->token);
  }
  else
  {
    printf("Can not print NULL struct \n");
  }
}

void list_print(_TOPIC_Q *ps )
{
  _TOPIC_NODE* p = NULL;

  if ( ps )
  {
    for ( p = ps->head; p; p = p->next )
    {
      list_print_element(p);
    }
  }

  printf("------------------\n");
}