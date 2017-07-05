#include"bptree.h"
#include<string.h>
#include<stdlib.h>
#include<limits.h>

Node* binary_search_node(Node *node, KEYT key){
	int start=0,end=node->count-2;
	while(1){
		int mid=(start+end)/2;
		if(mid==0 && node->separator[mid] > key)
			return node->children[mid].node;
		else if(node->separator[mid] <= key && node->separator[mid+1]>key)
			return node->children[mid+1].node;
		else if(mid==node->count-2 && node->separator[mid]<=key)
			return node->children[mid+1].node;
		else if(node->separator[mid] <key)
			start=mid+1;
		else if(node->separator[mid]>key)
			end=mid-1;
	}	
}

Node *node_init(Node *node){
	memset(node,0,sizeof(Node));
	return node;
}

Entry *make_entry(KEYT key,KEYT end,KEYT pbn1){
	Entry *res=(Entry*)malloc(sizeof(Entry));
	res->key=key;
	res->end=end;
	res->pbn=pbn1;
	if(pbn1>INT_MAX)
		sleep(10);
	res->version=0;
	res->parent=NULL;
	return res;
}
Entry *level_entry_copy(Entry *input){
	Entry *res=(Entry*)malloc(sizeof(Entry));
	res->key=input->key; res->pbn=input->pbn; res->end=input->end;
	res->version=input->version;
	res->parent=NULL;
	return res;
}
void free_entry(Entry *entry){
	free(entry);
}

level *level_init(level* input, int size){
	input->root=(Node*)malloc(sizeof(Node));
	input->root=node_init(input->root);
	input->root->leaf=true;
	input->root->parent=NULL;
	input->size=0;
	input->version=0;
	input->m_size=size;
	input->depth=0;
	return input;
}

Node *level_find_leafnode(level *lev, KEYT key){
	Node *temp=lev->root;
	while(!temp->leaf){
		temp=binary_search_node(temp,key);
	}
	return temp;
}

Entry *level_find(level *lev, KEYT key){
	Node *startN=level_find_leafnode(lev,0);
	Entry *temp=startN->children[0].entry;
	if(temp==NULL) return NULL;
	int cnt=1;
	while(1){
		if(temp->key <=key && temp->end>=key)
			return temp;
		if(cnt <startN->count){
			temp=startN->children[cnt++].entry;
		}
		else{
			startN=startN->children[MAXC].node;
			if(startN==NULL) break;
			temp=startN->children[0].entry;
			cnt=1;
		}
	}
	return NULL;
}

Entry **level_range_find(level *lev, KEYT start, KEYT end){
	Node *startN=level_find_leafnode(lev,start);
	Entry *temp=startN->children[0].entry;
	if(temp==NULL) return NULL;
	Entry **res=(Entry **)malloc(sizeof(Entry *)*(lev->m_size+1));
	int idx=0;
	int cnt=1;
	bool startingFlag=false;
	while(1){
		if(!startingFlag && !(temp->end < start || temp->key > end)){
			startingFlag=true;
			res[idx++]=temp;
		}
		else if(startingFlag){
			res[idx++]=temp;
		}
		if(cnt <startN->count){
			temp=startN->children[cnt++].entry;
		}
		else{
			startN=startN->children[MAXC].node;
			if(startN==NULL) break;
			temp=startN->children[0].entry;
			cnt=1;
		}
	}/*
		if(idx==0){
		free(res);
		return NULL;
		}*/
	res[idx]=NULL;
	return res;
}

Entry *level_get_victim(level *lev){
	Entry *temp=level_getFirst(lev);
	Entry *res;
	KEYT min=-1;
	int cnt=0;
	while(temp!=NULL){
		if(temp->key <min){
			res=temp;
			min=temp->key;
		}
		if(temp->parent==NULL)
			return temp;
		if(cnt<temp->parent->count){
			temp=temp->parent->children[cnt++].entry;
		}else{
			if(temp->parent->children[MAXC].node==NULL) break;
			temp=temp->parent->children[MAXC].node->children[0].entry;
			cnt=1;
		}
	}
	return res;
}


Node *level_directory_insert(level *lev,Node *target, KEYT sep, Node *prev, Node *next){
	while(1){
		if(target==NULL){
			Node *temp=(Node*)malloc(sizeof(Node));
			temp=node_init(temp);
			temp->separator[0]=sep;
			temp->children[0].node=prev;
			temp->children[1].node=next;
			temp->count=2;
			prev->parent=next->parent=temp;
			lev->root=temp;
			return temp;
		}
		next->parent=target;
		int idx_s=0;
		KEYT t_s[MAXC]={0,};
		Child t_c[MAXC+1]={0,};
		for(int i=0; i<target->count-1; i++){
			if(target->separator[i]<sep){ 
				t_s[idx_s++]=target->separator[i];
			}
			else{
				t_s[idx_s++]=sep;
				memcpy(&t_s[idx_s],&target->separator[i],sizeof(KEYT)*(target->count-1-i));
				memcpy(&t_c[0],&target->children[0],sizeof(Child)*(i+1));
				t_c[i+1].node=next;
				memcpy(&t_c[i+2],&target->children[i+1],sizeof(Child)*(target->count-i-1));
				break;
			}
			if(i==target->count-2){
				t_s[idx_s++]=sep;
				memcpy(&t_c[0],&target->children[0],sizeof(Child)*(i+2));
				t_c[i+2].node=next;
			}
		}
		target->count++;
		if(target->count<=MAXC){
			memcpy(&target->separator[0],&t_s[0],(target->count-1)*sizeof(KEYT));
			memcpy(&target->children[0],&t_c[0],(target->count)*sizeof(Child));
			return target;
		}
		int q=(MAXC+1)/2, r=(MAXC+1)%2;
		sep=t_s[q-1];

		Node *new_node=(Node*)malloc(sizeof(Node));
		new_node=node_init(new_node);
		memcpy(&new_node->separator[0],&t_s[q],sizeof(KEYT)*(MAXC-q));
		memset(&target->separator[0],0,sizeof(target->separator));
		memcpy(&target->separator[0],&t_s[0],sizeof(KEYT)*(q-1));

		memcpy(&new_node->children[0],&t_c[q],sizeof(Child)*(q+r));
		memset(&target->children[0],0,sizeof(target->children));
		memcpy(&target->children[0],&t_c[0],sizeof(Child)*(MAXC-q-r+1));

		target->count=q;
		new_node->count=q+r;
		new_node->parent=target->parent;
		for(int i=0; i<target->count; i++) target->children[i].node->parent=target;
		for(int i=0; i<new_node->count; i++)new_node->children[i].node->parent=new_node;

		prev=target;
		next=new_node;
		target=target->parent;
	}
}

Node *level_insert(level* lev, Entry *entry){
	Node *temp=level_find_leafnode(lev,entry->key);
	if(entry->pbn>INT_MAX){
		printf("??");
	}
	if(lev->size+1>lev->m_size) return NULL;
	if(entry->version==0) entry->version=lev->version++;
	if(temp->count==0){
		temp->separator[0]=entry->key;
		temp->children[0].entry=entry;
		lev->size++;
		temp->count++;
		return temp;
	}

	int place=0;
	for(int i=0; i<temp->count; i++){
		if(temp->children[i].entry->key<entry->key)
			place++;
		else
			break;
	}
	memcpy(&temp->children[place+1],&temp->children[place],sizeof(Child)*(temp->count-place));
	memcpy(&temp->separator[place+1],&temp->separator[place],sizeof(KEYT)*(temp->count-place));
	temp->separator[place]=entry->key;
	temp->children[place].entry=entry;
	entry->parent=temp;

	lev->size++;
	temp->count++;
	if(temp->count<MAXC) return temp;

	Node *next=(Node*)malloc(sizeof(Node));
	next=node_init(next);

	int q=(MAXC)/2, r=(MAXC)%2;
	memcpy(&next->separator[0],&temp->separator[q],sizeof(KEYT)*(q+r));
	memset(&temp->separator[q],0,sizeof(KEYT)*(q+r));
	memcpy(&next->children[0],&temp->children[q],sizeof(Child)*(q+r));
	memset(&temp->children[q],0,sizeof(Child)*(q+r));
	next->children[MAXC]=temp->children[MAXC];
	temp->children[MAXC].node=next;
	next->leaf=temp->leaf=true;
	next->count=q+r;
	temp->count=q;

	for(int i=0; i<next->count; i++) next->children[i].entry->parent=next;
	for(int i=0; i<temp->count; i++) temp->children[i].entry->parent=temp;

	if(level_directory_insert(lev,temp->parent,next->separator[0],temp,next)!=NULL) 
		return temp;
	return NULL;
}

Node *level_delete_restructuring(level *lev, Node *target){
	while(1){
		if(target->count> MAXC/2 || target->parent==NULL) return target;
		int idx=0;
		Node *next,*parent=target->parent;
		for(int i=0; i<parent->count; i++){
			if(parent->children[i].node==target){
				if(i==0){ next=parent->children[i+1].node; idx=i+1;}
				else{
					next=target;
					target=parent->children[i-1].node;
					idx=i;
				}
				break;
			}
		}
		KEYT t_s[2*MAXC]={0,};
		Child t_c[2*MAXC]={0,};
		memcpy(&t_c[0],&target->children[0],sizeof(Child)*target->count);
		memcpy(&t_c[target->count],&next->children[0],sizeof(Child)*next->count);

		memcpy(&t_s[0],&target->separator[0],sizeof(KEYT)*(target->count-1));
		t_s[target->count-1]=parent->separator[idx-1];
		memcpy(&t_s[target->count],&next->separator[0],sizeof(KEYT)*(next->count-1));

		int num=target->count+next->count;
		if(num>MAXC){
			int q=num/2,r=num%2;
			memset(&target->children[0],0,sizeof(Child)*MAXC);
			memcpy(&target->children[0],&t_c[0],sizeof(Child)*q);
			memset(&target->separator[0],0,sizeof(target->separator));
			memcpy(&target->separator[0],&t_s[0],sizeof(KEYT)*(q-1));

			parent->separator[idx-1]=t_s[q-1];

			memset(&next->children[0],0,sizeof(Child)*MAXC);
			memcpy(&next->children[0],&t_c[q],sizeof(Child)*(q+r));
			memset(&next->separator[0],0,sizeof(next->separator));
			memcpy(&next->separator[0],&t_s[q],sizeof(KEYT)*(q+r-1));

			target->count=q; next->count=q+r;
			for(int i=0; i<q; i++) target->children[i].node->parent=target;
			for(int i=0; i<q+r; i++)next->children[i].node->parent=next;
			return target;
		}
		else{
			memset(&target->children[0],0,sizeof(Child)*MAXC);
			memcpy(&target->children[0],&t_c[0],sizeof(Child)*num);

			memset(&target->separator[0],0,sizeof(target->separator));
			memcpy(&target->separator[0],&t_s[0],sizeof(KEYT)*(num-1));
			target->count=num;
			for(int i=0; i<num; i++) target->children[i].node->parent=target;

			free(next);
			memcpy(&parent->separator[idx-1],&parent->separator[idx],sizeof(KEYT)*(parent->count-1-idx));
			memcpy(&parent->children[idx],&parent->children[idx+1],sizeof(Child)*(parent->count-idx-1));
			parent->separator[parent->count-2]=0;
			parent->children[parent->count-1].node=NULL;
			--parent->count;

			if(parent->parent==NULL && parent->count==1){
				lev->root=target;
				free(parent);
				target->parent=NULL;
			}
			else
				target=parent;
		}
	}
}

Node * level_delete(level *lev, KEYT key){
	if(lev->size==0) return NULL;
	Node *temp=level_find_leafnode(lev,key);
	int idx=0;
	if(key){
		for(int i=0; i<temp->count; i++){
			if(temp->children[i].entry->key==key){
				free_entry(temp->children[i].entry);
				memcpy(&temp->children[i],&temp->children[i+1],sizeof(Child)*(temp->count-i));
				memcpy(&temp->separator[i],&temp->separator[i+1],sizeof(KEYT)*(temp->count-i));
				temp->children[MAXC-2].entry=NULL;
				temp->separator[MAXC-2]=0;
				--temp->count;
				idx=i;
				break;
			}
			if(i==temp->count-1) return NULL;
		}
	}
	else{
		for(int i=0; i<temp->count; i++){
			if(temp->children[i].entry->key || (i==0 && temp->children[i].entry->key > key)){
				free_entry(temp->children[i].entry);
				memcpy(&temp->children[i],&temp->children[i+1],sizeof(Child)*(temp->count-i));
				memcpy(&temp->separator[i],&temp->separator[i+1],sizeof(KEYT)*(temp->count-i));
				temp->children[MAXC-2].entry=NULL;
				temp->separator[MAXC-2]=0;
				--temp->count;
				idx=i;
				break;
			}
			if(i==temp->count-1) return NULL;
		}
	}

	if(temp->parent!=NULL && idx==0){
		for(int i=0; i<temp->parent->count-1; i++){
			if(temp->parent->separator[i]==key){
				temp->parent->separator[i]=temp->children[0].entry->key;
				break;
			}
		}
	}
	--lev->size;

	if(temp->count>=MAXC/2 || temp->parent==NULL) return temp;
	Node *next;
	for(int i=0; i<temp->parent->count; i++){
		if(temp==temp->parent->children[i].node){
			if(i==0) next=temp->children[MAXC].node;
			else{
				next=temp;
				temp=temp->parent->children[i-1].node;
			}
		}
	}
	Child t_c[2*MAXC]={0,};
	KEYT t_s[2*MAXC]={0,};
	memcpy(&t_c[0],&temp->children[0],sizeof(Child)*(temp->count));
	memcpy(&t_c[temp->count],&next->children[0],sizeof(Child)*(next->count));

	memcpy(&t_s[0],&temp->separator[0],sizeof(KEYT)*(temp->count));
	memcpy(&t_s[temp->count],&next->separator[0],sizeof(KEYT)*(next->count));
	int num=temp->count+next->count;
	if(num>MAXC-1){
		int q=num/2,r=num%2;
		memset(&temp->children[0],0,sizeof(Child)*MAXC);
		memcpy(&temp->children[0],&t_c[0],sizeof(Child)*q);
		memset(&temp->separator[0],0,sizeof(temp->separator));
		memcpy(&temp->separator[0],&t_s[0],sizeof(KEYT)*q);

		memset(&next->children[0],0,sizeof(Child)*MAXC);
		memcpy(&next->children[0],&t_c[q],sizeof(Child)*(q+r));
		memset(&next->separator[0],0,sizeof(next->separator));
		memcpy(&next->separator[0],&t_s[q],sizeof(KEYT)*(q+r));

		for(int i=0; i<next->parent->count; i++){
			if(next->parent->children[i].node==next){
				if(i==0)break;
				next->parent->separator[i-1]=next->separator[0];
				break;
			}
		}
		temp->count=q; next->count=q+r;
		for(int i=0; i<temp->count; i++) temp->children[i].entry->parent=temp;
		for(int i=0; i<next->count; i++) next->children[i].entry->parent=next;
		return temp;
	}
	else{
		memset(&temp->children[0],0,sizeof(Child)*MAXC);
		memcpy(&temp->children[0],&t_c[0],sizeof(Child)*num);
		memset(&temp->separator[0],0,sizeof(temp->separator));
		memcpy(&temp->separator[0],&t_s[0],sizeof(KEYT)*num);
		temp->count=num;

		temp->children[MAXC]=next->children[MAXC];
		for(int i=0; i<next->parent->count; i++){
			if(next->parent->children[i].node==next){
				memcpy(&next->parent->separator[i-1],&next->parent->separator[i],sizeof(KEYT)*((next->parent->count-1)-i));
				memcpy(&next->parent->children[i],&next->parent->children[i+1],sizeof(Child)*((next->parent->count)-i-1));
				next->parent->separator[next->parent->count-2]=0;
				next->parent->children[next->parent->count-1].node=NULL;
				next->parent->count--;
				free(next);
				break;
			}
		}

		for(int i=0; i<temp->count; i++) temp->children[i].entry->parent=temp;
		if(temp->parent->parent==NULL && temp->parent->count==1){
			lev->root=temp;
			free(temp->parent);
			temp->parent=NULL;
			return temp;
		}
		return level_delete_restructuring(lev,temp->parent);
	}
}

Entry *level_getFirst(level *lev){
	Node *temp=level_find_leafnode(lev,0);
	return temp->children[0].entry;
}
void level_free(level *lev){
	while(level_delete(lev,0)!=NULL){
	};
	free(lev->root);
	free(lev);
}
#ifdef DEBUG_B
#include<stdio.h>
#include<time.h>
int main(){
	level *lev=(level*)malloc(sizeof(level));
	lev=level_init(lev,1000);
	for(int i=1; i<1001; i++){
		level_insert(lev,make_entry(i,i,i));
	}
	Entry *res;
	for(int i=0; i<900; i++){
		int key=rand();
		key%=1000;
		key++;
		level_delete(lev,key);
	}
	printf("test\n");

	Entry *temp=level_getFirst(lev);
	int cnt=1;
	while(temp!=NULL){
		printf("%lld\n",temp->key);
		if(cnt<temp->parent->count){
			temp=temp->parent->children[cnt++].entry;
		}
		else{
			if(temp->parent->children[MAXC].node==NULL) break;
			temp=temp->parent->children[MAXC].node->children[0].entry;
			cnt=1;
		}
	}
	level_free(lev);
}
#endif
/*
   bpIterator *range_find_iterator(level *lev, BKEYT start, BKEYT end){
   bpIterator *iter=(bpIterator*)malloc(sizeof(bpIterator));
   iter->start=level_find_leafnode(lev,start);
   iter->end=level_find_leafnode(lev,end);
   iter->now=iter->start;
   iter->num=0;
   iter->startK=start; iter->endK=end;
   return iter;
   }

   Node *level_delete_by_iter(level *lev,bpIterator** iter){
   Entry *temp=bp_getNext(*iter);
   if(temp==NULL) return NULL;
   Node *res=level_delete(lev,temp->key);
   bpIterator *tempI=*iter;
 *iter=range_find_iterator(lev,(*iter)->startK,(*iter)->endK);
 free(tempI);
 return res;
 }*/

