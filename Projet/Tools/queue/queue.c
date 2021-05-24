#include "queue.h"

queue* queue_push(queue* start, void *ptr)
{
	queue *q = malloc(sizeof(queue));
	if(q == NULL)
		err(EXIT_FAILURE, "Error while creating the element");
	
	q->ptr = ptr;

	if(start != NULL)
	{
		q->next = start->next;
		start->next = q;
	}
	else
		q->next = q;
		

	return q;
}

queue* queue_pop(queue* start, void **ptr)
{
	if(start == NULL)
		err(EXIT_FAILURE, "Error while poping element, list is empty");
	

	queue *old = start->next;
	*ptr = old->ptr;

	if(start->next == start)
		start = NULL;
	else
		start->next = old->next;
	
	free(old);

	return start;
}

void queue_empty(queue** pstart)
{
	void *ptr = NULL;
    while(*pstart != NULL)
	{
		*pstart =  queue_pop(*pstart, &ptr);
	}
	   
}
