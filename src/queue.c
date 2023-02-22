#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "queue.h"

// NOTE: i was stoned when i wrote the thread safety for this so its probably not very good
// TODO: add error handling for failed mutex acquisitions

#define IMIN(a,b) ({typeof(a) _a = a, _b = b;(_a < _b) ? _a : _b;})

void Queue_init(Queue *queue) {
	queue->length = 0;

	queue->_head = NULL;
	queue->_tail = NULL;

	mtx_init(&queue->_head_lock, mtx_plain);
	mtx_init(&queue->_tail_lock, mtx_plain);
}

void Queue_free(Queue *queue) {
	for (struct QueueNode *ptr=queue->_head; ptr != NULL; ptr = queue->_head) {
		queue->_head = ptr->_next;
		free(ptr);
	}

	mtx_destroy(&queue->_head_lock);
	mtx_destroy(&queue->_tail_lock);
}

/*
 * Helper function which allocates a new node
 */
void _Queue_alloc_node(Queue *queue) {
	struct QueueNode *newNode = malloc(sizeof(struct QueueNode));

	newNode->_prev = queue->_tail;
	newNode->_next = NULL;
	newNode->length = 0;
	newNode->_head = 0;

	if (queue->_tail == NULL) {
		// _last being null means _first is NULL or invalid, so we have to set it
		queue->_head = newNode;
	} else {
		// link last node to this one
		queue->_tail->_next = newNode;
	}

	// Update the queue's last node ptr to point to this node
	queue->_tail = newNode;
}

void Queue_enqueue(Queue *queue, char value) {
	bool isTailLocked = false;

	if (queue->_tail == NULL || queue->_tail->length == (QUEUE_NODE_SIZE - queue->_tail->_head)) {
		// Last node does not exist or is full; we need to make a new one
		// lock both head and tail, as both will be modified
		if (mtx_lock(&queue->_head_lock) == thrd_success
		 && mtx_lock(&queue->_tail_lock) == thrd_success) {
			_Queue_alloc_node(queue);

			// Unlock head since we no longer need the head lock
			// We still need the tail lock, so indicate that its already locked
			mtx_unlock(&queue->_head_lock);
			isTailLocked = true;
		}
	}

	// Insert the value to the appropriate index and increment the length
	// ensure that tail is locked
	if (isTailLocked || mtx_lock(&queue->_tail_lock) == thrd_success) {
		// Wrtie new value to tail
		queue->_tail->_data[queue->_tail->length] = value;

		// unlock tail
		mtx_unlock(&queue->_tail_lock);
	}

	// atomically increase the queue's length
	++queue->_tail->length;
	++queue->length;
}


void Queue_enqueue_all(Queue *queue, size_t n, char *values) {
	bool isTailLocked = false;

	for (size_t i=0; i < n; ) {
		// Alloc node if needed
		if (queue->_tail == NULL
		 || queue->_tail->length == (QUEUE_NODE_SIZE - queue->_tail->_head)) {
			// Last node does not exist or is full; we need to make a new one
			// lock both head and tail, as both will be modified
			if (mtx_lock(&queue->_head_lock) == thrd_success
			 && mtx_lock(&queue->_tail_lock) == thrd_success) {
				_Queue_alloc_node(queue);

				// Unlock head since we no longer need the head lock
				// We still need the tail lock, so indicate that its already locked
				mtx_unlock(&queue->_head_lock);
				isTailLocked = true;
			}
		}

		// figure out how much space is left in this node
		// copy min(remaining_space, n-i) values into this node
		// add the number of copied values to i

		// Calculate the number of bytes to copy; either the maximum remaining
		// space or the total remaining bytes, whichever is less
		uint8_t nbytes = IMIN(QUEUE_NODE_SIZE - queue->_tail->length, n-i);

		// ensure that tail is locked
		if (isTailLocked || mtx_lock(&queue->_tail_lock) == thrd_success) {
			// Copy nbytes to the appropriate location
			memcpy(
				   &queue->_tail->_data[queue->_tail->length],
				   &values[i],
				   nbytes
			);

			// Unlock tail
			mtx_unlock(&queue->_tail_lock);
		}

		// Increment i by the number of copied bytes
		i += nbytes;

		queue->_tail->length += nbytes;
		queue->length += nbytes;
	}

}

char Queue_dequeue(Queue *queue) {
	if (queue->_head == NULL || queue->_head->length == 0) {
		// No values to dequeue so return nothing
		// yeah i know this isnt a great way to report errors but since this is
		// a specialized implementation for this application, and silently
		// returning zero wont hurt anything i think its fine
		return 0;
	}

	char val;

	// Lock head and read front value
	if (mtx_lock(&queue->_head_lock) == thrd_success) {
		val = queue->_head->_data[queue->_head->_head];
	}

	// Reduce the length and advance the head
	// This way we dont have to use memmove
	--queue->_head->length;
	--queue->length;
	++queue->_head->_head;

	if (queue->_head->length == 0) {
		// the first node in the queue is now empty
		if (queue->_head == queue->_tail) {
			// Theres only one node in the queue; reset the node
			queue->_head->_head = 0;
		} else {
			// Theres more than one node in the queue. Free this node and delete it
			queue->_head = queue->_head->_next;
			free(queue->_head->_prev);
			queue->_head->_prev = NULL;
		}
	}

	mtx_unlock(&queue->_head_lock);

	return val;
}

