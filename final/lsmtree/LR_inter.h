#ifndef __LR_INTER_H__
#define __LR_INTER_H__

#include<stdint.h>
#include<pthread.h>
#include"request.h"
#include"skiplist.h"
#include"utils.h"
/**request type***/
#define LR_READ_T 	32
#define LR_WRITE_T 	8
#define LR_COMP_T	4
#define LR_FLUSH_T	16
#define LR_DW_T		1
#define LR_DR_T		3
#define LR_DDR_T	0
#define LR_DDW_T    2
/***/
typedef struct sktable sktable;
typedef struct skiplist skiplist;
typedef struct keyset keyset;
typedef struct req_t req_t;
typedef struct lsmtree_gc_req_t{
	req_t *req;//always NULL
	void *params[4];
	int8_t (*end_req)(struct lsmtree_gc_req_t*);
	uint64_t now_number;
	uint64_t target_number;
	uint64_t seq_number;
	pthread_mutex_t meta_lock;
	sktable *res;
    keyset *keys;
	struct lsmtree_gc_req_t *parent;

	sktable *compt_headers;
	skiplist * skip_data;
}lsmtree_gc_req_t;

typedef struct lsmtree_req_t{
	req_t *req;
	void *params[4];
	int8_t (*end_req)(struct lsmtree_req_t*);	
	uint64_t now_number;
	uint64_t target_number;
	uint64_t seq_number;
	pthread_mutex_t meta_lock;
	sktable *res;
    keyset *keys;
	struct lsmtree_req_t *parent;
	
	char *padding[2];
}lsmtree_req_t;

int8_t lr_make_req(req_t *);
int8_t lr_end_req(lsmtree_req_t *);
int8_t lr_gc_make_req(int8_t);
int8_t lr_gc_end_req(lsmtree_gc_req_t *);
int8_t lr_req_wait(lsmtree_req_t *);
int8_t lr_inter_init();
int8_t lr_inter_free();
int8_t lr_is_supbusy();
int8_t lr_is_gc_needed();
#endif
