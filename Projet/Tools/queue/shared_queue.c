#include "shared_queue.h"

shared_queue* shared_queue_new()
{
	shared_queue *q = malloc(sizeof(shared_queue));

	if(q == NULL)
		err(EXIT_FAILURE, "Error while make malloc");

	q->queue = NULL;
	
	if(sem_init(&q->lock, 0, 1) < 0)
		err(EXIT_FAILURE, "Error while creating sem lock");
	if(sem_init(&q->size, 0, 0) < 0)
		err(EXIT_FAILURE, "Error while creating sem size");
	
	return q;
}

void shared_queue_push(shared_queue* sq, void *ptr)
{
	if(sem_wait(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while locking");

	sq->queue = queue_push(sq->queue, ptr);

	if(sem_post(&sq->size) < 0)
		err(EXIT_FAILURE, "Error while increasing size");
	if(sem_post(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while unlocking");
	
}

void *shared_queue_pop(shared_queue* sq)
{
	void *res = NULL;

	if(sem_wait(&sq->size) < 0)
		err(EXIT_FAILURE, "Error while decreasing size");
	if(sem_wait(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while locking");
	
	sq->queue = queue_pop(sq->queue, &res);

	if(sem_post(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while unlocking");
	
	return res;
}


int shared_queue_length(shared_queue* sq)
{
	int value;
	sem_getvalue(&sq->size, &value);
	return value;
}
int shared_queue_isEmpty(shared_queue* sq)
{
	return shared_queue_length(sq) == 0;
}

void shared_queue_destroy(shared_queue* sq)
{
	if(sem_wait(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while locking for destroy");

	queue_empty(&sq->queue);

	if(sem_destroy(&sq->size) < 0)
		err(EXIT_FAILURE, "Error while destroying sem size");
	if(sem_destroy(&sq->lock) < 0)
		err(EXIT_FAILURE, "Error while destroying sem lock");
	
	free(sq);
}
