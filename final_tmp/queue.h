/*
 * queue.h
 *
 *  Created on: 2011. 4. 23.
 *      Author: Chwang
 */

#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct _node {
	void *value;
	struct _node* next;
}node;

typedef node* nptr;

typedef struct _queue{
	int count;
	nptr front;
	nptr rear;
}queue;

queue* create_queue(void);
void destroy_queue(queue* qptr);
void add_rear(queue* qptr, void *value);
void* remove_front(queue*);

#endif /* QUEUE_H_ */
