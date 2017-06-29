#include "utils.h"
#include<stdio.h>
#ifdef DEBUG
#include<stdio.h>
int main(){
	int cnt=0;
//	printf("%d %d\n",sizeof(KEYT),sizeof(BKEYT));
	for(int i=0; i<10; i++){
		BKEYT t=MAKEKEY(i);
		printf("%llu %u\n",t,getKey(t));
		if(i!=getKey(t))
			cnt++;
		printf("%u\n",gettimeFromBKey(t));
	}
	printf("error : %d\n",cnt);
}
#endif

