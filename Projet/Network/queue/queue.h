#ifndef QUEUE
#define QUEUE

#include "../message.h"

typedef struct queue
{
    // Value of an element.
    MESSAGE *message;

    // Pointer to the next element.
    struct queue *next;
} queue;

// Pushes a new value onto a queue.
// start = Starting address of the queue.
// val = Value to push.
// Returns the new starting address of the queue.
queue* queue_push(queue* start, MESSAGE *message);

// Pops a value off a queue.
// start = Starting address of the queue.
// pval = Pointer used to return the value.
// Returns the new starting address of the queue.
queue* queue_pop(queue* start, MESSAGE **message);

// Removes all the elements of a queue.
// pstart = Address that contains the starting address of the queue.
// Must set the starting address to NULL.
void queue_empty(queue** pstart);

#endif