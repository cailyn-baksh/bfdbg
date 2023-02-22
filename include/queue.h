#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stddef.h>
#include <stdint.h>
#include <threads.h>


#define QUEUE_NODE_SIZE	64

/*
 * A node in the queue. Each queue node is an array of elements. The array
 * itself is fixed-length, however the length member of the node determines
 * how much of the array is valid data.
 *
 * _prev		A pointer to the previous node, or NULL if this is the first
 * _next		A pointer to the next node, or NULL if this is the last
 * length		The length of the array (i.e. how much of data is valid data)
 * _head		The offset for the start of the data. 
 * _data		The data in the array
 */
struct QueueNode {
	struct QueueNode *_Atomic _prev;
	struct QueueNode *_Atomic _next;

	_Atomic uint8_t length;
	_Atomic uint8_t _head;
	char _data[QUEUE_NODE_SIZE];
};

/*
 * A queue, implemented as a linked list of QueueNode structures.
 *
 * length		The total length of the queue. This is equal to the sum of
 * 				every node's length member.
 * _head		Pointer to the head node. If this is NULL, then _tail must
 * 				also be NULL.
 * _tail		Pointer to the tail node
 *
 * _head_lock	Mutex locking head modification. If the same thread is trying to
 * 				lock both the head and the tail simultaneously, then the head
 * 				lock must be acquired first to avoid deadlock.
 * _tail_lock	Mutex locking tail modification.
 */
typedef struct {
	_Atomic size_t length;

	struct QueueNode *_Atomic _head;
	struct QueueNode *_Atomic _tail;

	mtx_t _head_lock;
	mtx_t _tail_lock;
} Queue;

/*
 * Initializes a queue.
 *
 * queue		The queue to initialize
 */
void Queue_init(Queue *queue);

/*
 * Frees a queue
 *
 * queue		The queue to free
 */
void Queue_free(Queue *queue);

/*
 * Enqueues a value into a queue
 *
 * queue		The queue to add to
 * value		The value to enqueue
 */
void Queue_enqueue(Queue *queue, char value);

/*
 * Enqueues multiple values into a queue
 *
 * queue		The queue to add to
 * n			The number of values to be added
 * values		The values to add
 */
void Queue_enqueue_all(Queue *queue, size_t n, char *values);

/*
 * Dequeues a value from a queue
 *
 * queue		The value to remove from
 *
 * Returns the dequeued value. Returns 0 if no value could be removed.
 */
char Queue_dequeue(Queue *queue);

#endif  // _QUEUE_H_

