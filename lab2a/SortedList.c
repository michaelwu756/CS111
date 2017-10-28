#include <stdlib.h>
#include <sched.h>
#include "SortedList.h"
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
  struct SortedListElement *cur=list;
  int added=0;
  while(!added)
  {
    if(cur->next->key==NULL || *(cur->next->key)>*(element->key))
    {
      element->prev=cur;
      element->next=cur->next;
      cur->next->prev=element;
      if(opt_yield & INSERT_YIELD)
        sched_yield();
      cur->next=element;
      added=1;
    }  
    else
      cur=cur->next;
  }
}
int SortedList_delete( SortedListElement_t *element)
{
  element->next->prev=element->prev;
  if(opt_yield & DELETE_YIELD)
    sched_yield();
  element->prev->next=element->next;
  free(element);
}
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
  struct SortedListElement *cur=list->next;
  while(cur!=list)
  {
    if(cur->key!=NULL && *(cur->key)==*key)
      return cur;
    else
    {
      struct SortedListElement *next = cur->next;
      if(opt_yield & LOOKUP_YIELD)
        sched_yield();
      cur=next;
    }
  }
  return NULL;
}
int SortedList_length(SortedList_t *list)
{
  if(list==NULL || list->next==NULL || list->prev==NULL || list->key!=NULL)
    return -1;
  struct SortedListElement *cur=list;
  int elements=0;
  while(cur->next->prev==cur)
  {
    struct SortedListElement *next = cur->next;
    if(opt_yield & LOOKUP_YIELD)
      sched_yield();
    cur=next;
    if(cur->next==NULL || (cur->key==NULL && cur!=list))
      return -1;
    if(cur->key == NULL)
      return elements;
    else
      elements++;
  }
  return -1;
}
