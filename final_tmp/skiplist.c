#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include"utils.h"
#include"skiplist.h"
#include"LR_inter.h"
#include"threading.h"
#ifdef ENABLE_LIBFTL
#include "libmemio.h"
extern memio_t* mio;
#endif
extern threadset processor;
extern MeasureTime mt;
#ifdef THREAD
pthread_mutex_t dfd_lock;
#endif
KEYT ppa=0;
snode *snode_init(snode *node){
	node->value=(char*)malloc(PAGESIZE);
	for(int i=0; i<MAX_L; i++)
		node->list[i]=NULL;
	return node;
}

skiplist *skiplist_init(skiplist *point){
	point->level=1;
	point->start=-1;
	point->end=point->size=0;
	point->header=(snode*)malloc(sizeof(snode));
	point->header->list=(snode**)malloc(sizeof(snode)*(MAX_L+1));
	for(int i=0; i<MAX_L; i++) point->header->list[i]=point->header;
	point->header->key=-1;
	return point;
}
snode *skiplist_pop(skiplist *list){
	if(list->size==0) return NULL;
	KEYT key=list->header->list[1]->key;
	int i;
	snode *update[MAX_L+1];
	snode *x=list->header;
	for(i=list->level; i>=1; i--){
		update[i]=list->header;
	}
	x=x->list[1];
	if(x->key ==key){
		for(i =1; i<=list->level; i++){
			if(update[i]->list[i]!=x)
				break;
			update[i]->list[i]=x->list[i];
		}

		while(list->level>1 && list->header->list[list->level]==list->header){
			list->level--;
		}
		list->size--;
		return x;
	}
	return NULL;	
}
snode *skiplist_find(skiplist *list, KEYT key){
	if(list==NULL) return NULL;
	if(list->size==0) return NULL;
	snode *x=list->header;
	for(int i=list->level; i>=1; i--){
		while(x->list[i]->key<key)
			x=x->list[i];
	}
	if(x->list[1]->key==key)
		return x->list[1];
	return NULL;
}
static snode * skiplist_find_level(KEYT key, int level, skiplist *list){
	snode *temp=list->header;
	int _level=level;
	while(temp->list[_level]->key<key){
		temp=temp->list[_level];
	}
	return temp;
}
static int getLevel(){
	int level=1;
	int temp=rand();
	while(temp%4==1){
		temp=rand();
		level++;
		if(level+1>=MAX_L) break;
	}
	return level;
}
snode *skiplist_insert(skiplist *list,KEYT key, char *value, lsmtree_req_t* req,bool flag){
	snode *update[MAX_L+1];
	snode *x=list->header;
	if(list->start > key )list->start=key;
	if(list->end < key) list->end=key;
	for(int i=list->level; i>=1; i--){
		while(x->list[i]->key<key)
			x=x->list[i];
		update[i]=x;
	}
	x=x->list[1];

	if(key==x->key){
		if(value!=NULL)
			memcpy(x->value,value,SNODE_SIZE);
		return x;
	}
	else{
		int level=getLevel();
		if(level>list->level){
			for(int i=list->level+1; i<=level; i++){
				update[i]=list->header;
			}
			list->level=level;
		}

		x=(snode*)malloc(sizeof(snode));
		x->list=(snode**)malloc(sizeof(snode*)*(level+1));
		x->key=key;
		if(value !=NULL){
			x->value=value;
			x->req=req;
		}
		for(int i=1; i<=level; i++){
			x->list[i]=update[i]->list[i];
			update[i]->list[i]=x;
		}
		x->level=level;
		list->size++;
	}
	return x;
}

void skiplist_dump(skiplist *list){
	for(int i=list->level; i>=1; i--){
		printf("level dump - %d\n",i);
		snode *temp=list->header->list[i];
		while(temp!=list->header){
			printf("%lu ",temp->key);
			temp=temp->list[i];
		}
		printf("\n\n");
	}
}
void skiplist_ex_value_free(skiplist *list){
	free(list->header->list);
	free(list->header);
	free(list);
}
void skiplist_meta_free(skiplist *list){
	snode *temp=list->header->list[1];
	while(temp!=list->header){
		snode *next_node=temp->list[1];
		free(temp->list);
		free(temp);
		temp=next_node;
	}
	skiplist_ex_value_free(list);
}
void skiplist_free(skiplist *list){
	snode *temp=list->header->list[1];
	while(temp!=list->header){
		snode *next_node=temp->list[1];
		free(temp->list);
		free(temp);
		temp=next_node;
	}
	skiplist_ex_value_free(list);
}

void skiplist_sktable_free(sktable *f){
	free(f->value);
	free(f);
}
sktable *skiplist_read(KEYT pbn, int hfd, int dfd){
	sktable *res=(sktable*)malloc(sizeof(sktable));
	return res;
}
sktable *skiplist_meta_read(KEYT pbn, int fd,int seq,lsmtree_req_t *req){
	lsmtree_req_t *temp=(lsmtree_req_t*)malloc(sizeof(lsmtree_req_t));
	uint8_t *type=(uint8_t*)malloc(sizeof(uint8_t));
	*type=LR_DR_T;
	temp->req=NULL;
	temp->params[0]=(void*)type;
#ifndef ENABLE_LIBFTL
	temp->keys=(keyset*)malloc(PAGESIZE);
#else
	temp->dmatag=memio_alloc_dma(5,&(char*)temp->keys);
#endif
    temp->seq_number=seq;
	temp->end_req=req->end_req;
	temp->parent=req;

#ifndef ENABLE_LIBFTL
	pthread_mutex_lock(&dfd_lock);
	int number=0;
	if((number=lseek64(fd,((off64_t)PAGESIZE)*(pbn),SEEK_SET))==-1){
		printf("lseek error in meta read!\n");
		sleep(10);
	}
	read(fd,temp->keys,PAGESIZE);
	pthread_mutex_unlock(&dfd_lock);
	temp->end_req(temp);
#else
	memio_comp_read(mio,pbn,(uint64_t)(PAGESIZE),(uint8_t)temp->keys,1,temp,temp->dmatag);
#endif
	return NULL;
}

sktable* skiplist_data_read(sktable *list, KEYT pbn, int fd){/*
																lseek(fd,SKIP_BLOCK*pbn,SEEK_SET);
																list->value=malloc(KEYN*PAGESIZE);
																read(fd,list->value,KEYN*PAGESIZE);*/
	return NULL;
}

keyset *skiplist_keyset_find(sktable *t, KEYT key){
	short now;
	KEYT start=0,end=KEYN/2-1;
	KEYT mid=t->meta[1][0].key;
	if(key<mid){
		now=0;
	}
	else{
		now=1;
	}
	mid=(start+end)/2;
	while(1){
		if(start>end) return NULL;
		if(start==end){
			if(key==t->meta[now][mid].key)
				return &t->meta[now][mid];
			else
				return NULL;
		}
		if(key==t->meta[now][mid].key)
			return &t->meta[now][mid];
		else if(key<t->meta[now][mid].key){
			if(mid==0)
				return NULL;
			end=mid-1;
			mid=(start+end)/2;
		}
		else if(key> t->meta[now][mid].key){
			start=mid+1;
			mid=(start+end)/2;
		}
	}
}

bool skiplist_keyset_read(keyset* k,char *res,int fd,lsmtree_req_t *req){
	if(k==NULL)
		 return false;
	lsmtree_req_t *temp=(lsmtree_req_t*)malloc(sizeof(lsmtree_req_t));
	uint8_t *type=(uint8_t*)malloc(sizeof(uint8_t));
	*type=LR_DDR_T;
	temp->req=NULL;
	temp->params[0]=(void*)type;
	temp->end_req=req->end_req;
	temp->parent=req;
#ifndef ENABLE_LIBFTL
	pthread_mutex_lock(&dfd_lock);
	if(lseek64(fd,((off64_t)PAGESIZE)*(k->ppa),SEEK_SET)==-1)
		printf("lseek error in meta read!\n");
	read(fd,res,PAGESIZE);
	pthread_mutex_unlock(&dfd_lock);
	temp->end_req(temp);
#else
	memio_read(mio,k->ppa,(uint64_t)(PAGESIZE),(uint8_t)res,1,temp,temp->req->dmaTag);
#endif
	return 1;
}
KEYT skiplist_write(skiplist *data, lsmtree_gc_req_t * req,int hfd,int dfd){
	skiplist_data_write(data,dfd,req);
	skiplist_meta_write(data,hfd,req);
	return 0;
}
KEYT skiplist_meta_write(skiplist *data,int fd, lsmtree_gc_req_t *req){
	lsmtree_gc_req_t *temp_req;
	uint8_t *type;
	int now=0;
	int i=0;
	snode *temp=data->header->list[1];
	while(temp!=data->header){
		temp_req=(lsmtree_gc_req_t*)malloc(sizeof(lsmtree_gc_req_t));
		type=(uint8_t*)malloc(sizeof(uint8_t));
		*type=LR_DW_T;
#ifndef ENABLE_LIBFTL
		temp_req->keys=(keyset*)malloc(PAGESIZE);
#else
		temp_req->dmatag=memio_alloc_dma(4,(char**)(&(temp_req->keys)));
#endif
		temp_req->parent=req;
		temp_req->params[0]=(void*)type;
		temp_req->end_req=req->end_req;
		for(int i=0; i<KEYN/2; i++){
			temp_req->keys[i].key=temp->key;
			temp_req->keys[i].ppa=temp->ppa;
			temp=temp->list[1];
		}
#ifndef ENABLE_LIBFTL
		pthread_mutex_lock(&dfd_lock);
		if(lseek64(fd,((off64_t)PAGESIZE)*(ppa++),SEEK_SET)==-1)
			printf("lseek error in meta read!\n");
		write(fd,temp_req->keys,PAGESIZE);
		pthread_mutex_unlock(&dfd_lock);
		temp_req->end_req(temp_req);
#else
		memio_comp_write(mio,ppa++,(uint64_t)(PAGESIZE),(uint8_t)temp_req->keys,1,(lsmtree_req_t*)temp_req,temp_req->dmatag);
#endif
	}

	return ppa-2;
}
KEYT skiplist_data_write(skiplist *data,int fd,lsmtree_gc_req_t * req){
	KEYT where=ppa;
	lsmtree_req_t *child_req;
	snode *temp=data->header->list[1];
	req->skip_data=data;
	while(temp!=data->header){
        child_req=temp->req;
		*((uint8_t*)child_req->params[0])=LR_DDW_T;
		child_req->end_req=lr_end_req;
		child_req->parent=(lsmtree_req_t*)req;
		temp->ppa=ppa;
#ifndef ENABLE_LIBFTL
		pthread_mutex_lock(&dfd_lock);
		if(lseek64(fd,((off64_t)PAGESIZE)*(ppa),SEEK_SET)==-1)
			printf("lseek error in meta read!\n");

		write(fd,temp->value,PAGESIZE);
		pthread_mutex_unlock(&dfd_lock);
		child_req->end_req(child_req);
#else
		memio_write(mio,ppa,(uint64_t)(PAGESIZE),(uint8_t)temp->value,1,child_req,child_req->req->dmaTag);
#endif
		temp=temp->list[1];
		ppa++;
	}
	return 0;
}

skiplist *skiplist_cut(skiplist *list,KEYT num){
	if(list->size<num) return NULL;
	skiplist *res=(skiplist*)malloc(sizeof(skiplist));
	res=skiplist_init(res);
	snode *h=res->header;
	res->start=-1;
	for(KEYT i=0; i<num; i++){
		snode *temp=skiplist_pop(list);
		if(temp==NULL) return NULL;
		res->start=temp->key>res->start?res->start:temp->key;
		res->end=temp->key>res->end?temp->key:res->end;
		h->list[1]=temp;
		temp->list[1]=res->header;
		h=temp;
	}
	res->size=num;
	return res;
}
#ifdef DEBUG

int main(){
	skiplist * temp;

	char data[SNODE_SIZE]={0,};
	int dfd=open("data/test_data.skip",O_CREAT|O_RDWR|O_TRUNC,0666);
	int hfd=open("data/test_header.skip",O_CREAT|O_RDWR|O_TRUNC,0666);
	for(int i=0; i<2; i++){
		temp=(skiplist*)malloc(sizeof(skiplist));
		skiplist_init(temp);
		for(int j=0+i*KEYN; j<(i+1)*KEYN; j++){
			memcpy(data,&j,sizeof(j));
			skiplist_insert(temp,j,data,true);
		}
		skiplist_write(temp,i,hfd,dfd);
		skiplist_free(temp);
	}

	sktable *t;
	for(int i=0; i<2; i++){
		t=skiplist_read(i,hfd,dfd);
		for(int j=0; j<KEYN; j++){
			printf("%d\n",t->meta[j].key);
		}
	}
}
#endif
