#include"LR_inter.h"
#include"lsmtree.h"
#include"threading.h"
#include"utils.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
lsmtree *LSM;
threadset processor;
MeasureTime mt;
extern pthread_mutex_t sst_lock;
extern pthread_mutex_t mem_lock;
int8_t lr_inter_init(){
	LSM=(lsmtree*)malloc(sizeof(lsmtree));
	threadset_init(&processor);
	threadset_start(&processor);
	
	measure_init(&mt);
	if(init_lsm(LSM)!=NULL)
		return 0;
	return -1;
}
int8_t lr_inter_free(){
	threadset_end(&processor);
	threadset_clear(&processor);
	lsm_free(LSM);
	return 0;	
}
int8_t lr_gc_make_req(int8_t t_num){
	uint8_t *type=(uint8_t*)malloc(sizeof(uint8_t));
	lsmtree_gc_req_t * gc_req;
	while(1){
		switch(t_num){
			case 0:
				*type=LR_FLUSH_T;
				pthread_mutex_lock(&sst_lock);
				if(LSM->sstable !=NULL){
					pthread_mutex_unlock(&sst_lock);
					continue;
				}
				LSM->sstable=LSM->memtree;

				pthread_mutex_lock(&mem_lock);
				LSM->memtree=(skiplist*)malloc(sizeof(skiplist));
				LSM->memtree=skiplist_init(LSM->memtree);
				pthread_mutex_unlock(&mem_lock);

				pthread_mutex_unlock(&sst_lock);
				gc_req=(lsmtree_gc_req_t*)malloc(sizeof(lsmtree_gc_req_t));
				gc_req->params[0]=(void*)type;
				gc_req->params[1]=NULL;
				gc_req->params[2]=(void*)LSM->sstable;
				gc_req->params[3]=(void*)LSM;
				gc_req->end_req=lr_gc_end_req;
				threadset_gc_assign(&processor,gc_req);
				break;
			default:
				*type=LR_COMP_T;
				gc_req=(lsmtree_gc_req_t*)malloc(sizeof(lsmtree_gc_req_t));
				gc_req->params[0]=(void*)type;
				gc_req->params[1]=(void*)LSM->buf.disk[t_num-1];
				gc_req->params[2]=(void*)LSM->buf.disk[t_num];
				gc_req->params[3]=(void*)LSM;
				gc_req->end_req=lr_gc_end_req;
				threadset_gc_assign(&processor,gc_req);
				break;
		}
		break;
	}
	return 0;
}
int8_t lr_make_req(req_t *r){
	uint8_t *type=(uint8_t*)malloc(sizeof(uint8_t));
	int test_num;
	switch(r->type_info->type){
		case 3:
		case 1://set
			*type=LR_WRITE_T;
			break;
		case 2://get
			*type=LR_READ_T;
			break;
		default:
			return -1;
			break;
	}
	lsmtree_req_t *th_req=(lsmtree_req_t*)malloc(sizeof(lsmtree_req_t));
	th_req->req=r;
	th_req->end_req=lr_end_req;
	th_req->params[0]=(void*)type;
	th_req->params[1]=(void*)&r->key_info->key;
	th_req->params[2]=(void*)r->value_info->value;
	th_req->params[3]=(void*)LSM;
	threadset_assign(&processor,th_req);
	return 0;
}

int8_t lr_end_req(void *ra){
	lsmtree_req_t *r=(lsmtree_req_t *)ra;
	lsmtree_req_t *parent;
	uint8_t *req_type=(uint8_t*)r->params[0];
	switch(*req_type){
		case LR_DR_T:
			parent=r->parent;
			pthread_mutex_lock(&parent->meta_lock);
			parent->now_number++;
			if(r->seq_number==0)
				memcpy(parent->res->meta[0],r->keys,PAGESIZE);
			else
				memcpy(parent->res->meta[1],r->keys,PAGESIZE);
			pthread_mutex_unlock(&parent->meta_lock);
#ifdef ENABLE_LIBFTL
			memio_free_dma(5,r->dmatag);
#else
			free(r->keys);
#endif
			break;
		case LR_DDR_T:
			parent=r->parent;
			pthread_mutex_lock(&parent->meta_lock);
			parent->now_number++;
			pthread_mutex_unlock(&parent->meta_lock);
			break;
		case LR_DDW_T:
			parent=r->parent;
			pthread_mutex_lock(&parent->meta_lock);
			parent->now_number++;
			pthread_mutex_unlock(&parent->meta_lock);
			break;
		case LR_READ_T:
			pthread_mutex_destroy(&r->meta_lock);
			break;
		default:
			break;
	}

	free((uint8_t*)r->params[0]);
	if(r->req!=NULL){
		r->req->end_req(r->req);
	}
	free(r);
	return 0;
}
int8_t lr_gc_end_req(void *ra){
	lsmtree_gc_req_t *r=(lsmtree_gc_req_t*)ra;
	lsmtree_gc_req_t *parent;
	uint8_t *req_type=(uint8_t*)r->params[0];
	int setnumber,offset;
	switch(*req_type){
		case LR_DW_T:
			parent=r->parent;
			pthread_mutex_lock(&parent->meta_lock);
			parent->now_number++;
			pthread_mutex_unlock(&parent->meta_lock);
#ifdef ENABLE_LIBFTL
			memio_free_dma(4,r->dmatag);
#else
			free(r->keys);
#endif
			break;
		case LR_DR_T:
			parent=r->parent;
			setnumber=(r->seq_number/2);
			offset=r->seq_number%2;
			pthread_mutex_lock(&parent->meta_lock);
			parent->now_number++;
			memcpy(&parent->compt_headers[setnumber].meta[offset],r->keys,PAGESIZE);
			pthread_mutex_unlock(&parent->meta_lock);
#ifdef ENABLE_LIBFTL
			memio_free_dma(5,r->dmatag);
#else
			free(r->keys);
#endif
			break;
		case LR_FLUSH_T:			
			lr_req_wait((lsmtree_req_t*)r);
			pthread_mutex_destroy(&r->meta_lock);
			skiplist_free(r->skip_data);
			break;
		case LR_COMP_T:
			lr_req_wait((lsmtree_req_t *)r);
			pthread_mutex_destroy(&r->meta_lock);
			if(r->compt_headers!=NULL)
				free(r->compt_headers);
			break;
	}
	free((uint8_t*)r->params[0]);
	free(r);
}
int8_t lr_req_wait(lsmtree_req_t *input){
	while(1){
		pthread_mutex_lock(&input->meta_lock);
		if(input->now_number==input->target_number){
			pthread_mutex_unlock(&input->meta_lock);
			break;
		}
		pthread_mutex_unlock(&input->meta_lock);
	}
	return 0;
}
int8_t lr_is_gc_needed(){
	if(is_flush_needed(LSM))
		return 0;
	for(int i=LEVELN-1; i>=0; i--){
		if(LSM->buf.disk[i]==NULL)
			break;
		if(LSM->buf.disk[i]->size>=LSM->buf.disk[i]->m_size){
			return i+1;
		}
	}
	return -1;
}
int8_t lr_is_supbusy(){
	pthread_mutex_lock(&processor.gc_lock);
	if(processor.gc_q->count>THREADQN*BUSYPOINT){
		pthread_mutex_unlock(&processor.gc_lock);
		return 1;
	}
	pthread_mutex_unlock(&processor.gc_lock);
	return 0;
}
