#include"bptree.h"
#include"lsmtree.h"
#include"skiplist.h"
#include"measure.h"
#include"queue.h"
#include<time.h>
#include<pthread.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/time.h>

#ifdef ENABLE_LIBFTL
#include "libmemio.h"
memio_t* mio;
#endif


static int filenumber=0;
int debugkey=0;
extern int memtable_get_count;
extern int readbuffer_get_count;
extern int sstable_get_count;
extern struct timeval compaction_MAX;
extern struct MeasureTime mt;

#ifdef THREAD
extern int thread_cnt;
extern pthread_t read_thread[THREADNUM];
extern pthread_mutex_t buf_header_lock;
extern pthread_mutex_t activate_check[THREADNUM];
extern pthread_mutex_t th_cnt;
extern int isactivated[THREADNUM];
#ifdef DEBUG_THREAD
extern pthread_mutex_t debug_m;
#endif
pthread_mutex_t header_read_argv_check;
pthread_mutex_t qlock;
sem_t consum;
KEYT argv[3];

Queue *q;
void *thread_header_read(void *input){
	KEYT pbn=argv[0];
	int fd=argv[1];
	int thread_n=argv[2];
	pthread_mutex_unlock(&header_read_argv_check);

	pthread_mutex_lock(&th_cnt);
	thread_cnt++;
	pthread_mutex_unlock(&th_cnt);

#ifdef DEBUG_THREAD
	pthread_mutex_lock(&debug_m);
	printf("[header read]pbn: %d num:%d start\n",pbn,thread_n);
	pthread_mutex_unlock(&debug_m);
#endif

	sktable *sk=skiplist_meta_read(pbn,fd);
	pthread_mutex_lock(&qlock);
	enqueue(q,sk);
	sem_post(&consum);
	pthread_mutex_unlock(&qlock);

#ifdef DEBUG_THREAD
	pthread_mutex_lock(&debug_m);
	printf("[header read]pbn: %d num:%d end\n",pbn,thread_n);
	pthread_mutex_unlock(&debug_m);
#endif

	pthread_mutex_lock(&th_cnt);
	thread_cnt--;
	pthread_mutex_unlock(&th_cnt);

	pthread_mutex_lock(&activate_check[thread_n]);
	isactivated[thread_n]=0;
	pthread_mutex_unlock(&activate_check[thread_n]);
}
#endif
int where_am_i;
int sstcheck;
extern uint64_t ppa;
int write_data(lsmtree *LSM,skiplist *data){
	skiplist_data_write(data,LSM->dfd);
	uint64_t resfilenumber=ppa;
	skiplist_meta_write(data,LSM->dfd);
	return resfilenumber;
}
int write_meta_only(lsmtree *LSM, skiplist *data){
	snode *check=data->header->list[1];
	uint64_t resfilenumber=ppa;
	skiplist_meta_write(data,LSM->dfd);
	return resfilenumber;
}
lsmtree* init_lsm(lsmtree *res){
	measure_init(&mt);
	res->memtree=(skiplist*)malloc(sizeof(skiplist));
	res->memtree=skiplist_init(res->memtree);
	res->buf.data=NULL;

	for(int i=0;i<LEVELN; i++){
		res->buf.disk[i]=NULL;
		res->buf.lastB=NULL;
	}

#if !defined(ENABLE_LIBFTL)
	if(SEQUENCE){
		res->dfd=open("data/skiplist_data.skip",O_RDWR|O_CREAT|O_TRUNC,0666);
	}
	else{
		res->dfd=open("data/skiplist_data_r.skip",O_RDWR|O_CREAT|O_TRUNC,0666);
	}
	if(res->dfd==-1 || res->dfd==-1){
		printf("file open error!\n");
		return NULL;
	}
#endif
#ifdef ENABLE_LIBFTL
	if ( (mio = memio_open() ) == NULL ) {
		printf("memio_open() failed\n");
		return NULL;
	}
#endif
	return res;
}
void buffer_free(buffer *buf){
	free(buf->data);
	for(int i=0; i<LEVELN; i++){
		if(buf->disk[i]!=NULL) level_free(buf->disk[i]);
	}
	if(buf->lastB!=NULL)
		skiplist_meta_free(buf->lastB);
}

void lsm_free(lsmtree *input){
	skiplist_free(input->memtree);
	buffer_free(&input->buf);
#if !defined(ENABLE_LIBFTL)
	close(input->dfd);
#endif
#ifdef ENABLE_LIBFTL
	memio_close(mio);
#endif
	free(input);
}
void *lsm_clear(lsmtree *input){
	skiplist_free(input->memtree);
	input->memtree=(skiplist*)malloc(sizeof(skiplist));
	input->memtree=skiplist_init(input->memtree);
}
bool put(lsmtree *LSM,KEYT key, char *value){
	if(key==0)
		printf("key is 0!\n");
	debugkey=key;
	while(1){
		if(LSM->memtree->size<KEYN){
			skiplist_insert(LSM->memtree,key,value,true);
			return true;
		}
		else{
			merge(LSM,0);
		}
	}
	return false;
}

int get(lsmtree *LSM,KEYT key,char *ret){
	memset(ret,0,PAGESIZE);
	int tempidx=0;
	snode* res=skiplist_find(LSM->memtree,key);
	if(res!=NULL){
		memcpy(ret,res->value,PAGESIZE);
		return 1;
	}
#ifdef THREAD
		pthread_mutex_lock(&buf_header_lock);
#endif
	if(LSM->buf.data!=NULL){

		if(skiplist_keyset_read(skiplist_keyset_find(LSM->buf.data,key),ret,LSM->dfd)){
#ifdef THREAD
			pthread_mutex_unlock(&buf_header_lock);
#endif
			return 1;
		}
	}
#ifdef THREAD
		pthread_mutex_unlock(&buf_header_lock);
#endif

	for(int i=0; i<LEVELN; i++){
		if(LSM->buf.disk[i]!=NULL){
			Entry *temp=level_find(LSM->buf.disk[i],key);
			if(temp==NULL) continue;
			sktable *readed=skiplist_meta_read(temp->pbn,LSM->dfd);
			if(skiplist_keyset_read(skiplist_keyset_find(readed,key),ret,LSM->dfd)){
				
#ifdef THREAD	
				pthread_mutex_lock(&buf_header_lock);
#endif
				if(LSM->buf.data!=NULL){
					free(LSM->buf.data);
				}
				LSM->buf.data=readed;
#ifdef THREAD
				pthread_mutex_unlock(&buf_header_lock);
#endif
				return 1;
			}
			free(readed);
		}
	}

	res=NULL;
	res=skiplist_find(LSM->buf.lastB,key);
	if(res!=NULL){
		keyset read_temp;
		read_temp.key=res->key;
		read_temp.ppa=res->ppa;
		skiplist_keyset_read(&read_temp,ret,LSM->dfd);
		return 1;
	}
	return 0;
}
lsmtree* lsm_reset(lsmtree* input){
	input->memtree=(skiplist*)malloc(sizeof(skiplist));
	input->memtree=skiplist_init(input->memtree);
	return input;
}
bool compaction(lsmtree *LSM,level *src, level *des,int n){
	Entry *target;
	if(src==NULL){
		target=make_entry(LSM->memtree->start,LSM->memtree->end,write_data(LSM,LSM->memtree));
		lsm_clear(LSM);
	}
	else{
		target=level_get_victim(src);
		Entry *target2=level_entry_copy(target);
		level_delete(src,target->key);
		target=target2;
	}
	skiplist *last=LSM->buf.lastB;
	if(last==NULL){
		last=(skiplist*)malloc(sizeof(skiplist));
		skiplist_init(last);
	}

	Entry **iter=level_range_find(des,target->key,target->end);
	Entry *temp;
	bool check_getdata=false;

	if(iter==NULL)
		goto next;
	int versionIdx=0;
	KEYT *version;
	KEYT *delete_set;
	version=(KEYT*)malloc(sizeof(KEYT)*(des->m_size));
	delete_set=(KEYT*)malloc(sizeof(KEYT)*(des->m_size));
	
#ifdef THREAD
	pthread_mutex_init(&header_read_argv_check,NULL);
	pthread_mutex_init(&qlock,NULL);
	sem_init(&consum,0,0);
	q=(Queue*)malloc(sizeof(Queue));
	queue_init(q,des->m_size);
	int allnumber=0;
#endif
	for(int i=0; iter[i]!=NULL ;i++){
		temp=iter[i];
		check_getdata=true;

		version[versionIdx]=temp->version;
		delete_set[versionIdx++]=temp->key;
#ifdef THREAD
		bool flag=false;
		allnumber++;
		while(1){
			pthread_mutex_lock(&th_cnt);
			if(thread_cnt<THREADNUM){
				pthread_mutex_unlock(&th_cnt);
				for(int j=0; j<THREADNUM; j++){
					pthread_mutex_lock(&activate_check[j]);
					if(isactivated[j]!=0){
						pthread_mutex_unlock(&activate_check[j]);
						continue;
					}
					else{
						isactivated[j]=1;
						pthread_mutex_unlock(&activate_check[j]);
						
						pthread_mutex_lock(&header_read_argv_check);
						argv[0]=temp->pbn;
						argv[1]=LSM->dfd;
						argv[2]=j;
						pthread_create(&read_thread[j],NULL,thread_header_read,NULL);
						pthread_detach(read_thread[j]);
						flag=true;
						break;
					}
				}
			}
			else
				pthread_mutex_unlock(&th_cnt);
			if(flag)break;
		}
#else
		sktable *sk=skiplist_meta_read(temp->pbn,LSM->dfd);
		for(int j=0; j<KEYN; j++){
			snode *temp_s=skiplist_insert(last,sk->meta[j].key,NULL,true);
			temp_s->ppa=sk->meta[j].ppa;
		}
		free(sk);
#endif
	}
#ifdef THREAD
	for(int i=0; i<allnumber; i++){
		sem_wait(&consum);
		pthread_mutex_lock(&qlock);
		sktable *sk=dequeue(q);
		pthread_mutex_unlock(&qlock);
		for(int j=0; j<KEYN; j++){
			snode *temp_s=skiplist_insert(last,sk->meta[j].key,NULL,true);
			temp_s->ppa=sk->meta[j].ppa;
		}
		free(sk);
	}
	pthread_mutex_destroy(&header_read_argv_check);
	pthread_mutex_destroy(&qlock);
	sem_destroy(&consum);
	queue_free(q);
#endif
	free(iter);

next:
	if(!check_getdata)
		level_insert(des,target);
	else{

		sktable *sk=skiplist_meta_read(target->pbn,LSM->dfd);
		for(int i=0; i<KEYN; i++){
			snode *temp_s=skiplist_insert(last,sk->meta[i].key,NULL,true);
			temp_s->ppa=sk->meta[i].ppa;
		}
		free(sk);
		free(target);

		int getIdx=0;
		skiplist *t;
		for(int i=0; i<versionIdx; i++){
			level_delete(des,delete_set[i]);
		}
		free(delete_set);

		while((t=skiplist_cut(last,KEYN))){
			temp=make_entry(t->start, t->end,write_meta_only(LSM,t));
			if(getIdx<versionIdx){
				temp->version=version[getIdx++];
			}
			level_insert(des,temp);
			skiplist_meta_free(t);
		}
		free(version);
	}

	LSM->buf.lastB=last;
	return true;
}
bool merge(lsmtree *LSM,int t){
	while(1){
		if(t==0){
			if(LSM->buf.disk[0]==NULL){
				LSM->buf.disk[0]=(level*)malloc(sizeof(level));
				level_init(LSM->buf.disk[0],MUL);
			}
			if(LSM->buf.disk[0]->size<LSM->buf.disk[0]->m_size){
				MS(&mt);
				compaction(LSM,NULL,LSM->buf.disk[0],0);
				where_am_i=1;
				ME(&mt,"level_0_1_comaction");
			}
			else{
				t++; continue;
			}
			return true;
		}
		else{
			if(LSM->buf.disk[t]==NULL){
				LSM->buf.disk[t]=(level*)malloc(sizeof(level));
				level_init(LSM->buf.disk[t],LSM->buf.disk[t-1]->size*MUL);
			}
			if(LSM->buf.disk[t]->size<LSM->buf.disk[t]->m_size){
				MS(&mt);
				compaction(LSM,LSM->buf.disk[t-1],LSM->buf.disk[t],t);
				where_am_i=t+1;
				char buf[100];
				sprintf(buf,"level_%d_%d_compaction",t,t+1);
				ME(&mt,buf);
				t--;
			}
			else{
				t++;
			}
			continue;
		}
	}
	return false;
}
//depcrated
void level_traversal(level* t){
	Node *temp=level_find_leafnode(t,0);

}
