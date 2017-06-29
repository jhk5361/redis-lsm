/*
 * queue.c
 *
 *  Created on: 2011. 4. 23.
 *      Author: Chwang
 */

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

queue* create_queue(){
	queue *new_queue=(queue*)malloc(sizeof(queue));
	new_queue->count=0;
	new_queue->front=NULL;
	new_queue->rear=NULL;
	return new_queue;
}

void destroy_queue(queue *qptr){
	while(qptr->count!=0){
		remove_front(qptr);
	}
	free(qptr);
	//printf("Queue Destroy Complete\n");
}

void add_rear(queue *qptr, void *value){
	nptr new_node=(nptr)malloc(sizeof(node));
	new_node->value=value;
	new_node->next=NULL;

	if (qptr->count==0){
		qptr->front=new_node;
		qptr->rear=new_node;
	}
	else{
		qptr->rear->next=new_node;
		qptr->rear=new_node;
	}

	qptr->count++;
}

void* remove_front(queue *qptr){
	if (qptr->count==0){
		//printf("There is no item in this queue.\n");
		return NULL;
	}
	else{
		nptr tmp=qptr->front;
		qptr->front=tmp->next;
		void *tmp_value=tmp->value;
		free(tmp);
		qptr->count--;
		return tmp_value;
	}
}
