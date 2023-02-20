#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define IMIN(a,b) ({typeof(a) _a = a, _b = b;(_a < _b) ? _a : _b;})

/*
 * To optimize enqueuing and dequeuing in cases where lots of data is being
 * enqueued and then dequeued (for example, when the user enters in a lengthy
 * piece of code and then runs it all at once), a spare QueueNode is kept
 * around instead of being freed after it has been allocated. Then, then a new
 * QueueNode is needed, this one is reinitialized instead of being malloc'd.
 * The intention of this optimization is to prevent the appropriate amount of
 * space from being allocated by the OS, then that memory being later returned
 * to the OS, reassigned to another process, and then the same amount of memory
 * being requested by this process immediately after (and thus forcing the OS
 * to find another block that can be assigned for this purpose).
 *
 * NOTE: i dont actually know if this is a worthwhile optimization; consider
 * NOTE: removing it in the future
 */
struct QueueNode *spare = NULL;

void Queue_init(Queue *queue) {
	queue->length = 0;

	queue->_first = NULL;
	queue->_last = NULL;
}

void Queue_free(Queue *queue) {

	for (struct QueueNode *ptr=queue->_first; ptr != NULL; ptr = queue->_first) {
		queue->_first = ptr->_next;
		free(ptr);
	}
}

/*
 * Helper function which allocates a new node
 */
void _Queue_alloc_node(Queue *queue) {
	struct QueueNode *newNode = malloc(sizeof(struct QueueNode));

	newNode->_prev = queue->_last;
	newNode->_next = NULL;
	newNode->length = 0;
	newNode->_head = 0;

	if (queue->_last == NULL) {
		// _last being null means _first is NULL or invalid, so we have to set it
		queue->_first = newNode;
	} else {
		// link last node to this one
		queue->_last->_next = newNode;
	}

	// Update the queue's last node ptr to point to this node
	queue->_last = newNode;
}

void Queue_enqueue(Queue *queue, char value) {
	if (queue->_last == NULL || queue->_last->length == QUEUE_NODE_SIZE) {
		// last node does not exist or is full; we need to make a new one
		_Queue_alloc_node(queue);
	}

	// Insert the value to the appropriate index and increment the length
	queue->_last->_data[queue->_last->length] = value;
	++queue->_last->length;
	++queue->length; 
}


void Queue_enqueue_all(Queue *queue, size_t n, char *values) {
	for (size_t i=0; i < n; ) {
		// Alloc node if needed
		if (queue->_last == NULL || queue->_last->length == QUEUE_NODE_SIZE) {
			// last node does not exist or is full; we need to make a new one
			_Queue_alloc_node(queue);
		}

		// figure out how much space is left in this node
		// copy min(remaining_space, n-i) values into this node
		// add the number of copied values to i

		// Calculate the number of bytes to copy; either the maximum remaining
		// space or the total remaining bytes, whichever is less
		uint8_t nbytes = IMIN(QUEUE_NODE_SIZE - queue->_last->length, n-i);

		// Copy nbytes to the appropriate location
		memcpy(
			   &queue->_last->_data[queue->_last->length],
			   &values[i],
			   nbytes
		);

		// Increment i by the number of copied bytes
		i += nbytes;

		queue->_last->length += nbytes;
		queue->length += nbytes;
	}

}

char Queue_dequeue(Queue *queue) {
	if (queue->_first == NULL || queue->_first->length == 0) {
		// No values to dequeue so return nothing
		// yeah i know this isnt a great way to report errors but since this is
		// a specialized implementation for this application, and silently
		// returning zero wont hurt anything i think its fine
		return 0;
	}

	char val = queue->_first->_data[queue->_first->_head];

	// Reduce the length and advance the head
	// This way we dont have to use memmove
	--queue->_first->length;
	--queue->length;
	++queue->_first->_head;

	if (queue->_first->length == 0) {
		// the first node in the queue is now empty
		if (queue->_first == queue->_last) {
			// Theres only one node in the queue; reset the node
			queue->_first->_head = 0;
		} else {
			// Theres more than one node in the queue. Free this node and delete it
			queue->_first = queue->_first->_next;
			free(queue->_first->_prev);
			queue->_first->_prev = NULL;
		}
	}

	if (queue->_first->length == 0 && queue->_first != queue->_last) {
		// the first node in the queue is now empty and there is more than one
		// node in the queue. Free this node and delete it
	}

	return val;
}

