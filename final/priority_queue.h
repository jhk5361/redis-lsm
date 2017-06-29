#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__
typedef struct {
	int priority;
	void *data;
} node_t;

typedef struct {
	node_t *nodes;
	int len;
	int size;
} heap_t;

void PQ_push(heap_t*, unsigned int, void*);
void* PQ_pop(heap_t*);
node_t* PQ_front(heap_t*);
heap_t* create_PQ();
void destroy_PQ(heap_t*);

#endif
