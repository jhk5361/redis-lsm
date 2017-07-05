#ifndef __SKIPLIST_HEADER
#define __SKIPLIST_HEADER
#define MAX_L 30
#include"utils.h"
#include"stdint.h"
#include"LR_inter.h"
typedef struct lsmtree_req_t lsmtree_req_t;
typedef struct lsmtree_gc_req_t lsmtree_gc_req_t;
typedef struct snode{
	KEYT key;
	KEYT ppa;
	int level;
	char *value;
	struct lsmtree_req_t *req;
	struct snode **list;
}snode;

typedef struct skiplist{
	uint8_t level;
	KEYT start,end;
	uint64_t size;
	snode *header;
}skiplist;

typedef struct skIterator{
	skiplist *mylist;
	snode *now;
} skIterator;

typedef struct keyset{
	KEYT key;
	KEYT ppa;
}keyset;

typedef struct sktable{
	keyset meta[2][KEYN/2];
	char *value;
}sktable;

snode *snode_init(snode*);
skiplist *skiplist_init(skiplist*);
snode *skiplist_find(skiplist*,KEYT);
snode *skiplist_insert(skiplist*,KEYT,char *,struct lsmtree_req_t *,bool);

sktable *skiplist_read(KEYT, int hfd, int dfd);
sktable *skiplist_meta_read(KEYT, int fd,int ,struct lsmtree_req_t *);
sktable *skiplist_data_read(sktable*,KEYT pbn, int fd);
keyset* skiplist_keyset_find(sktable *,KEYT key);
bool skiplist_keyset_read(keyset* ,char *,int fd,lsmtree_req_t *);
void skiplist_sktable_free(sktable *);

snode *skiplist_pop(skiplist *);
KEYT skiplist_write(skiplist*,lsmtree_gc_req_t *,int hfd, int dfd);
KEYT skiplist_meta_write(skiplist *, int fd,struct lsmtree_gc_req_t*);
KEYT skiplist_data_write(skiplist *, int fd,struct lsmtree_gc_req_t*);
skiplist* skiplist_cut(skiplist *,KEYT num);
void skiplist_ex_value_free(skiplist *list);
void skiplist_meta_free(skiplist *list);
void skiplist_free(skiplist *list);
skIterator* skiplist_getIterator(skiplist *list);
void skiplist_traversal(skiplist *data);
#endif
