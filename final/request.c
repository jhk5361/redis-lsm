#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "priority_queue.h"
#include "request.h"
#include "command.h"

#ifdef ENABLE_LIBFTL
#include "libmemio.h"
#endif

pthread_mutex_t queue_mutx = PTHREAD_MUTEX_INITIALIZER;
extern queue *req_queue;

req_t *reqs[1000] = {0,};
int start = 0, end = 0;

int end_req(req_t*);

void EnqueReq(req_t *req, uint64_t seq) {
//	req->key |= seq << 32; // make unique key
	pthread_mutex_lock(&queue_mutx);
	printf("Type: %d, Key: %"PRIu64"\nValue len:%d\nValue: %s\n",req->type_info->type,req->key_info->key,req->value_info->len,req->value_info->value);
	printf("enqueue!!! sock %d\n", req->fd);
	add_rear(req_queue, (void*)req);
	pthread_mutex_unlock(&queue_mutx);
}

req_t* DequeReq() {
	req_t *req;
	pthread_mutex_lock(&queue_mutx);
	req = (req_t*)remove_front(req_queue);
	pthread_mutex_unlock(&queue_mutx);
	return req;
}

#ifdef ENABLE_LIBFTL
void alloc_dma(req_t *req) {
	req->dmaTag = memio_alloc_dma(req->type_info->type, req->value_info->value);
	return;
}
#endif

#ifdef ENABLE_LIBFTL
void free_dma(req_t *req) {
	//memio_free_dma(req->type_info->type, req->value_info->value);
	memio_free_dma(req->type_info->type, req->dmaTag);
	return 0;
}
#endif

req_t* alloc_req(req_t *req) {
	req = (req_t*)malloc(sizeof(req_t));
	req->keyword_info = (Keyword_info*)malloc(sizeof(Keyword_info));
	req->type_info = (Type_info*)malloc(sizeof(Type_info));
	req->key_info = (Key_info*)malloc(sizeof(Key_info));
	req->value_info = (Value_info*)malloc(sizeof(Value_info));
	#ifndef ENABLE_LIBFTL
	req->value_info->value = (char*)malloc(sizeof(char)*8192);
	memset(req->value_info->value, 0, 8192);
	#endif
	req->fd = 0;
	req->keyword_info->valid = 0;
	req->type_info->valid = 0;
	req->key_info->valid = 0;
	req->value_info->valid = 0;

	req->type_info->offset = 0;
	req->key_info->offset = 0;
	req->value_info->offset = 0;

	req->keyword_info->keywordNum = 0;
	req->type_info->len = 0;
	req->key_info->len = 0;
	req->value_info->len = 0;
	
	req->end_req = end_req;

	memset(req->type_info->type_str, 0, 10);
	return req;	
}

void free_req(req_t *req) {
	#ifdef ENABLE_LIBFTL
	if ( req->type_info->type == 2) free_dma(req);
	#endif
	
	#ifndef ENABLE_LIBFTL
	free(req->value_info->value);
	#endif
	free(req->keyword_info);
	free(req->type_info);
	free(req->key_info);
	free(req->value_info);
	free(req);
	return;
}

void* proc_req(void *arg) {
	req_t *req;
	int i;
	printf("start\n");
	static int cnt = 0;

	while(1) {
		if( (req = DequeReq()) == NULL ) {
		//	printf("Deque failed\n");
			continue;
		}
		cnt++;
		switch(req->type_info->type) {
		case 1:		// SET
			//printf("SET\treq cnt: %d, clnt_sock : %d, Key : %"PRIu64"\n", cnt, req->fd, req->key);
			//reqs[start] = req;
			//start = ( start + 1 ) % 1000;
			SendOkCommand(req->fd);
			break;
		case 2:		// GET
			//printf("case 2\n");
			//printf("OGET\treq cnt: %d, clnt_sock : %d, Key : %"PRIu64"\n", cnt, req->fd, req->key);
			/*
			for ( i=0; i < 1000; i++ ) {
				if ( reqs[i] != 0 && reqs[i]->key_info->key == req->key_info->key ) {
					//printf("IGET\treq cnt: %d, clnt_sock : %d\n", cnt, req->fd);
					SendBulkValue(req->fd, req->value_info->value, req->value_info->len);
					break;
				}
			}*/
//			if(i == 1000)
			SendBulkValue(req->fd, NULL, -1);
			break;
		case 3:		// DEL
			break;
		}
	}
}

#ifdef ENABLE_LIBFTL
int make_req(req_t *req) {
	if ( lr_make_req(req) == -1 ) {
		SendBulkValue(req->fd, NULL, -1);
		printf("make req failed!\n");
		return -1;
	}
	if ( req->type_info->type == 1 ) {
		SendOkCommand(req->fd);
		return 0;
	}
}
#endif

//#ifdef ENABLE_LIBFTL
int end_req(req_t *req) {
	if ( req->type_info->type == 2 ) {
		unsigned int *_cur = req->cur;
		if ( req->seq == *_cur ) {
			node_t *n;
			req_t *r;
			SendBulkValue(req->fd, req->value_info->value, req->value_info->len);
			*_cur = ( *_cur + 1 ) % 0xffffffff;
			while ( (n = PQ_front(req->pq)) != NULL ) {
				r = (req_t*)(n->data);
				if ( r->seq == *_cur ) {
					PQ_pop(req->pq);
					SendBulkValue(r->fd, r->value_info->value, r->value_info->len);
					*_cur = ( *_cur + 1 ) % 0xffffffff;
					free_req(r);
				}
				else break;
			}
			free_req(req);
		}
		else {
			PQ_push(req->pq, req->seq, (void*)req);
		}
	}
	else {
		free_req(req);
	}
	return 0;
}
//#endif
