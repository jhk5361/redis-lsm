#include"threading.h"
#include"LR_inter.h"
#include"skiplist.h"
#include"lsmtree.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<inttypes.h>
#include<signal.h>
#include<errno.h>
#define __STDC_FORMAT_MACROS
extern pthread_mutex_t dfd_lock;
void threading_init(threading *input){
	pthread_mutex_init(&input->activated_check,NULL);
	pthread_mutex_init(&input->terminate,NULL);
	input->isactivated=false;
	input->terminateflag=false;
	input->buf_data=NULL;
}

void threading_clear(threading *input){
	pthread_mutex_destroy(&input->activated_check);
	pthread_mutex_destroy(&input->terminate);
	if(input->buf_data!=NULL)
		free(input->buf_data);
}
void threadset_init(threadset* input){
	pthread_mutex_init(&input->req_lock,NULL);
	pthread_mutex_init(&input->th_cnt_lock,NULL);
	pthread_mutex_init(&input->gc_lock,NULL);
#ifdef DEBUG_THREAD
	pthread_mutex_init(&input->debug_m,NULL);
#endif

	for(int i=0; i<THREADNUM;i ++){
		threading_init(&input->threads[i]);
	}
	threading_init(&input->gc_thread);
#ifdef DEUBG_THREAD
	input->counter=0;
	input->errcnt=0;
#endif
	input->activatednum=input->max_act=0;
	input->req_q=create_queue();
	input->gc_q=create_queue();
}

void threadset_clear(threadset *input){
	for(int i=0; i<THREADNUM; i++){
		threading_clear(&input->threads[i]);
	}
	threading_clear(&input->gc_thread);
	pthread_mutex_destroy(&input->req_lock);
	pthread_mutex_destroy(&input->th_cnt_lock);
	pthread_mutex_destroy(&input->gc_lock);
#ifdef DEBUG_THREAD
	pthread_mutex_destroy(&input->debug_m);
#endif
	destroy_queue(input->req_q);
	destroy_queue(input->gc_q);
}
void* thread_gc_main(void *input){
	int number=0;
	threadset *master=(threadset*)input;
	threading *myth=&master->gc_thread;
	while(1){
		pthread_mutex_lock(&master->gc_lock);
		lsmtree_gc_req_t *lsm_req=(lsmtree_gc_req_t*)remove_front(master->gc_q);
		if(lsm_req==NULL){
			pthread_mutex_lock(&myth->terminate);
			if(myth->terminateflag){
				pthread_mutex_unlock(&myth->terminate);
				pthread_mutex_unlock(&master->gc_lock);
				break;
			}
			pthread_mutex_unlock(&myth->terminate);
			pthread_mutex_unlock(&master->gc_lock);
			continue;
		}
		
		pthread_mutex_lock(&myth->activated_check);
		myth->isactivated=true;
		printf(" %d : activated!\n",number);
		pthread_mutex_unlock(&myth->activated_check);
		
		pthread_mutex_unlock(&master->gc_lock);

		pthread_mutex_lock(&myth->terminate);
		if(myth->terminateflag){
			pthread_mutex_unlock(&myth->terminate);
			break;
		}
		else{
			pthread_mutex_unlock(&myth->terminate);
			uint8_t *type=(uint8_t*)lsm_req->params[0];
			lsmtree *LSM=(lsmtree*)lsm_req->params[3];
			level* src,*des;
			skiplist* data;
			char *res;
			KEYT *key;
			Entry *result_entry;
			switch((*type)){
				case LR_COMP_T:
					src=(level*)lsm_req->params[1];
					des=(level*)lsm_req->params[2];
					lsm_req->now_number=0;
					lsm_req->target_number=0;
					pthread_mutex_init(&lsm_req->meta_lock,NULL);
					compaction(LSM,src,des,NULL,lsm_req);
					break;
				case LR_FLUSH_T:
					data=(skiplist*)lsm_req->params[2];
					lsm_req->now_number=0;
					lsm_req->target_number=KEYN+2;
					pthread_mutex_init(&lsm_req->meta_lock,NULL);
					result_entry=make_entry(data->start,data->end,write_data(LSM,data,lsm_req));
					compaction(LSM,NULL,LSM->buf.disk[0],result_entry,lsm_req);
					LSM->sstable=NULL;
					break;
				default:
					break;
			}
			lsm_req->end_req(lsm_req);

			pthread_mutex_lock(&myth->activated_check);
			myth->isactivated=false;
			printf(" %d : deactivated!\n",number++);
			pthread_mutex_unlock(&myth->activated_check);
		}
	}
	return NULL;
}
void* thread_main(void *input){
	threadset *master=(threadset*)input;
	int number;
	for(int i=0; i<THREADNUM; i++){
		if(master->threads[i].id==pthread_self()){
			number=i;
			break;
		}
	}
	threading *myth=&master->threads[number];
	while(1){
		pthread_mutex_lock(&master->req_lock);
		lsmtree_req_t *lsm_req=(lsmtree_req_t*)remove_front(master->req_q);	
		if(lsm_req==NULL){
			pthread_mutex_lock(&myth->terminate);
			if(myth->terminateflag){
				pthread_mutex_unlock(&myth->terminate);
				pthread_mutex_unlock(&master->req_lock);
				break;
			}
			pthread_mutex_unlock(&myth->terminate);
			pthread_mutex_unlock(&master->req_lock);
			continue;
		}
		pthread_mutex_lock(&myth->activated_check);
		myth->isactivated=true;
		pthread_mutex_unlock(&myth->activated_check);

		pthread_mutex_unlock(&master->req_lock);
		
		pthread_mutex_lock(&myth->terminate);
		if(myth->terminateflag){
			pthread_mutex_unlock(&myth->terminate);
			break;
		}
		else{
			pthread_mutex_unlock(&myth->terminate);
			uint8_t *type=(uint8_t*)lsm_req->params[0];
			KEYT *key=(KEYT*)lsm_req->params[1];
			char *value;		
			lsmtree *LSM=(lsmtree*)lsm_req->params[3];
			switch((*type)){
				case LR_READ_T:
					value=(char*)lsm_req->params[2];
					pthread_mutex_init(&lsm_req->meta_lock,NULL);
					if(!thread_get(LSM,*key,myth,value,lsm_req)){
						printf("%lu error\n",*key);
						sleep(10);
					}
					lsm_req->end_req(lsm_req);
					break;
				case LR_WRITE_T:
					value=(char*)lsm_req->params[2];
					put(LSM,*key,value,lsm_req);
					break;
				default:
					break;
			}
			pthread_mutex_lock(&myth->activated_check);
			myth->isactivated=false;
			pthread_mutex_unlock(&myth->activated_check);
		}
	}
	return NULL;
}

sktable *sk_from_ths(threadset* input){/*
	while(1){
		if(input->sk_now_number==input->sk_target_number){
			return NULL;
		}
		pthread_mutex_lock(&input->res_lock);
		if(input->res_q->count!=0){
			sktable *res=(sktable*)remove_front(input->res_q);
			pthread_mutex_unlock(&input->res_lock);
			input->sk_now_number++;
			return res;
		}
		else{
			pthread_mutex_unlock(&input->res_lock);
			continue;
		}
	}*/
	return NULL;
}
void threadset_start(threadset* input){
	for(int i=0; i<THREADNUM; i++){
		input->threads[i].number=i;
		pthread_create(&input->threads[i].id,NULL,&thread_main,(void*)input);	
	}
	pthread_create(&input->gc_thread.id,NULL,&thread_gc_main,(void*)input);
}

void threadset_assign(threadset* input, lsmtree_req_t *req){
	while(1){
		pthread_mutex_lock(&input->req_lock);
		if(input->req_q->count<THREADQN){
			add_rear(input->req_q,(void*)req);
			pthread_mutex_unlock(&input->req_lock);
			return;
		}
		else{
			pthread_mutex_unlock(&input->req_lock);
		}
	}
	return;
}

void threadset_gc_assign(threadset* input ,lsmtree_gc_req_t *req){
	while(1){
		pthread_mutex_lock(&input->gc_lock);
		if(input->gc_q->count<THREADQN){
			add_rear(input->gc_q,(void*)req);
			pthread_mutex_unlock(&input->gc_lock);
			return;
		}
		else{
			pthread_mutex_unlock(&input->gc_lock);
		}
	}
}
void threadset_gc_wait(threadset *input){
while(1){
		pthread_mutex_lock(&input->gc_lock);
		pthread_mutex_lock(&input->gc_thread.activated_check);
		if(input->gc_q->count!=0 || input->gc_thread.isactivated){
			pthread_mutex_unlock(&input->gc_thread.activated_check);
			pthread_mutex_unlock(&input->gc_lock);
			continue;
		}
		pthread_mutex_unlock(&input->gc_thread.activated_check);
		pthread_mutex_unlock(&input->gc_lock);
		break;
	}
}

void threadset_request_wait(threadset *input){
	while(1){
		pthread_mutex_lock(&input->req_lock);
		if(input->req_q->count!=0){
			bool flag=false;
			for(int i=0; i<THREADNUM; i++){
				pthread_mutex_lock(&input->threads[i].activated_check);
				if(input->threads[i].isactivated){
					flag=true;
					pthread_mutex_unlock(&input->threads[i].activated_check);
					break;
				}
				pthread_mutex_unlock(&input->threads[i].activated_check);
			}
			if(!flag){
				pthread_mutex_unlock(&input->req_lock);
				break;
			}
			pthread_mutex_unlock(&input->req_lock);
			continue;
		}
		pthread_mutex_unlock(&input->req_lock);
		break;
	}
}
void threadset_end(threadset *input){
	threadset_gc_wait(input);
	pthread_mutex_lock(&input->gc_thread.terminate);
	input->gc_thread.terminateflag=true;
	pthread_mutex_unlock(&input->gc_thread.terminate);
	pthread_join(input->gc_thread.id,NULL);

	threadset_request_wait(input);
	for(int i=0; i<THREADNUM; i++){
		pthread_mutex_lock(&input->threads[i].terminate);
		input->threads[i].terminateflag=true;
		pthread_mutex_unlock(&input->threads[i].terminate);
		pthread_join(input->threads[i].id,NULL);
	}
	return;
}
