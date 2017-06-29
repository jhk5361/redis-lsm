#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <inttypes.h>
#include "queue.h"
#include "request.h"
#include "command.h"

pthread_mutex_t queue_mutx = PTHREAD_MUTEX_INITIALIZER;
extern queue *req_queue;

req_t *reqs[1000] = {0,};
int start = 0, end = 0;

void EnqueReq(req_t* req, uint64_t seq) {
//	req->key |= seq << 32; // make unique key
	pthread_mutex_lock(&queue_mutx);
//	printf("Type: %d, Key: %"PRIu64"\nValue len:%d\nValue: %s\n",req->type_info->type,req->key_info->key,req->value_info->len,req->value_info->value);
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

void req_cb(int type, void *value) {

}
