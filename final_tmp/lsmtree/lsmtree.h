#ifndef __LSM_HEADER__
#define __LSM_HEADER__
#include"bptree.h"
#include"skiplist.h"
#include"threading.h"
#include"utils.h"
typedef struct threading threading;
typedef struct buffer{
	sktable *data;
	level *disk[LEVELN];
	skiplist *lastB;
}buffer;

typedef struct table{
	int lev_addr[LEVELN];
}table;
typedef struct lsmtree{
	skiplist *memtree;
	skiplist *sstable;
	buffer buf;
	table tlb;
	int dfd;
}lsmtree;
lsmtree* init_lsm(lsmtree *);
bool merge(lsmtree *,KEYT target, skiplist *);
bool put(lsmtree *,KEYT key, char* value,lsmtree_req_t *);
bool is_gc_needed(lsmtree *);
bool compaction(lsmtree *,level *, level *,Entry *,lsmtree_gc_req_t *);
int get(lsmtree *,KEYT key,char *);
int thread_get(lsmtree *,KEYT key,threading *, char *value,lsmtree_req_t *);
void lsm_free(lsmtree *);
int write_data(lsmtree *LSM,skiplist *,lsmtree_gc_req_t *);
bool is_flush_needed(lsmtree * );
bool is_compt_needed(lsmtree *,KEYT);
#endif
