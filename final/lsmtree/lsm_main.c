#include"LR_inter.h"
#include"measure.h"
#include"utils.h"
#include"threading.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
extern threadset processor;

int main(){
	lr_inter_init();
	req_t * req;
	KEYT key;
	int fd;
	if(SEQUENCE==0)
		fd=open("data/rand.txt",O_RDONLY);
	for(int i=1; i<=INPUTSIZE; i++){
		while(lr_is_supbusy()){}
		req=(req_t*)malloc(sizeof(req_t));
		req->type=1;
		if(SEQUENCE==0)
			read(fd,&key,sizeof(key));
		else{
			key=i;
		}
			req->key=key;
		req->value=(char*)malloc(PAGESIZE);
		memcpy(req->value,&key,sizeof(KEYT));
		lr_make_req(req);
	}
	if(SEQUENCE==0){
		close(fd);
		fd=open("data/rand.txt",O_RDONLY);
	}
	printf("throw all write req!\n");
	threadset_request_wait(&processor);
	for(int i=1; i<=INPUTSIZE; i++){
		while(lr_is_supbusy()){}
		req=(req_t*)malloc(sizeof(req_t));
		req->type=2;
		if(SEQUENCE==0)
			read(fd,&key,sizeof(key));
		else{
			key=i;
		}
		req->key=key;
		req->value=(char*)malloc(PAGESIZE);
		key=i;
		lr_make_req(req);
	}
	printf("throw all read req!\n");
	threadset_request_wait(&processor);
	lr_inter_free();
}
