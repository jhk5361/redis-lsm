#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdint.h>
#include <pthread.h>
#include "priority_queue.h"
//#define PAGESIZE 8192
/*
typedef struct{
	int fd;
	uint64_t key;
	char *value;
	char type;	// 1: SET 2:GET 3:DEL
	int len;
}req_t;
*/

typedef struct {
	int keywordNum;
	char valid;
}Keyword_info;

typedef struct {
	char type;
	char type_str[10];
	int offset;
	int len;
	char valid;
}Type_info;

typedef struct {
	uint64_t key;
	int offset;
	int len;
	char valid;
}Key_info;

typedef struct {
	char* value;
	int offset;
	int len;
	char valid;
}Value_info;

typedef struct {
	int fd;
	int dmaTag;
	unsigned int seq;
	unsigned int *cur;
	pthread_mutex_t *mutx;
	Keyword_info *keyword_info;
	Type_info *type_info;
	Key_info *key_info;
	Value_info *value_info;
}req_t;


void EnqueReq(req_t*, uint64_t);
req_t* DequeReq();
void alloc_dma(req_t*);
void free_dma(req_t*);
req_t* alloc_req(req_t*);
void free_req(req_t*);
void* proc_req(void*);
int make_req(req_t*);
int end_req(req_t*);

#endif
