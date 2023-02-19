#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stddef.h>
#include <stdint.h>

/*
 * Queue implementation optimized for this application
 *
 * Queue is implemented as a doubly linked list
 * each node in the list is an array that holds a fixed number of elements
 * thus, the overhead of the doubly linked list is balanced with the size of
 * the data elements
 *
 * each queue node keeps track of its head index. As values are dequeued, the
 * length is decremented and the head index is incremented. when length drops
 * to 0, the node is discarded and replaced by the next node. this avoids the
 * hassle of trying to shift values around in memory and carry them over
 * between nodes
 */

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
	struct QueueNode *_prev;
	struct QueueNode *_next;

	uint8_t length;
	uint8_t _head;
	char _data[QUEUE_NODE_SIZE];
};

/*
 * A queue, implemented as a linked list of QueueNode structures.
 *
 * length		The total length of the queue. This is equal to the sum of
 * 				every node's length member.
 * _first		The first node in the queue. If this is NULL, then _last must
 * 				also be NULL.
 * _last		The last node in the queue
 */
typedef struct {
	size_t length;

	struct QueueNode *_first;
	struct QueueNode *_last;
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

