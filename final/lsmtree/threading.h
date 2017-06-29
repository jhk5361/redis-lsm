#ifndef __THREAD_H__
#define __THREAD_H__
#include"lsmtree.h"
#include"utils.h"
#include"LR_inter.h"
#include"queue.h"
#include"skiplist.h"
#include<pthread.h>
#include<semaphore.h>
typedef struct threading{
	pthread_t id;
	pthread_mutex_t activated_check;
	pthread_mutex_t terminate;
	int number;
	sktable *buf_data;
	bool isactivated;
	bool terminateflag;
}threading;

typedef struct threadset{
	pthread_mutex_t th_cnt_lock;
	pthread_mutex_t res_lock;
	pthread_mutex_t req_lock;
	pthread_mutex_t gc_lock;
#ifdef DEBUG_THREAD
	pthread_mutex_t debug_m;
	int errcnt;
	int counter;
#endif
	threading threads[THREADNUM];
	threading gc_thread;
	queue *res_q;
	queue *req_q;
	queue *gc_q;
	int activatednum;
	int max_act;
	int sk_target_number;
	int sk_now_number;
}threadset;
void threading_clear(threading *);
void threading_init(threading *);
void threadset_init(threadset*);
void threadset_start(threadset*);
void threadset_assign(threadset*,lsmtree_req_t *);
void threadset_end(threadset*);
void threadset_clear(threadset*);
void threadset_request_wait(threadset*);
void threadset_gc_assign(threadset*,lsmtree_gc_req_t *);
void threadset_gc_wait(threadset*);
sktable *sk_from_ths(threadset*);
#endif
