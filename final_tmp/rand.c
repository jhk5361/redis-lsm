#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<time.h>
#include"utils.h"

int cmp(const void *a, const void *b){
	return (*(int*)a-*(int*)b);
}
KEYT keySet[INPUTSIZE];
int main(){
	int fd=open("data/rand.txt",O_CREAT|O_TRUNC|O_WRONLY,0666);
	int fd2=open("data/rand_s.txt",O_CREAT|O_TRUNC|O_WRONLY,0666);
	int fd3= open("data/rand_u.txt",O_CREAT|O_TRUNC|O_WRONLY,0666);
	if(fd==-1)
		printf("file open error!");
	char buf[10];
	srand(time(NULL));
	for(int i=0; i<INPUTSIZE; i++){
		keySet[i]=rand()%INPUTSIZE+1;
	}
	write(fd,keySet,sizeof(keySet));

	for(int i=1; i<=INPUTSIZE; i++){
		sprintf(buf,"%ld\n",keySet[i-1]);
		write(fd3,buf,strlen(buf));
		if(i%KEYN==0)
			write(fd3,"/\n",strlen("/\n"));
	}
	qsort(keySet,INPUTSIZE,sizeof(KEYT),cmp);

	for(int i=0; i<INPUTSIZE; i++){
		if(i!=0 && keySet[i]==keySet[i-1]) continue;
		sprintf(buf,"%ld\n",keySet[i]);
		write(fd2,buf,strlen(buf));
	}
}
