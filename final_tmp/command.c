#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>
#include "queue.h"
#include "request.h"
#include "priority_queue.h"
//for lsmtree
//#include "../lsmtree/utils.h"

/*
   if valid == 0 -> not yet
   else succ

   return -1 -> not yet
   else next postion to process
 */
int GetLength(char *str, char cmp, int len, int cur, req_t *req, char which) {
	int added_len=0;
	char flag = 0;
//	int length;
	if ( cur == len ) return -1;
	else if ( str[cur] == '\n' ) {
//		printf("GetLength1 :%c, %d\n",cmp,which);
		switch(which) {
			case 1:	// keyword
				req->keyword_info->valid++;
				break;
			case 2:	// type
				req->type_info->valid++;
				break;
			case 3:	// key
				req->key_info->valid++;
				break;
			case 4:	// value
				req->value_info->valid++;
				break;
		}
		return cur+1;
	}

	if ( cur < len && str[cur] == cmp ) cur++;
	if ( cur == len ) return -1;

	while ( cur < len && str[cur++] != '\r' ) {
		added_len = added_len * 10 + str[cur-1] - '0' ;
		//printf("added_len : %d, %c\n", added_len, str[cur-1]);

	}

	if ( cur < len && str[cur] == '\n' ) flag = 1;

	switch(which) {
		case 1:	// keyword
			req->keyword_info->keywordNum = req->keyword_info->keywordNum * 10 + added_len;
//			length = req->keyword_info->keywordNum;
			if ( flag ) req->keyword_info->valid++;
			break;
		case 2:	// type
			req->type_info->len = req->type_info->len * 10 + added_len;
			if ( flag ) req->type_info->valid++;
			break;
		case 3:	// key
		//printf("old key %d\n",req->key_info->len);
			req->key_info->len = req->key_info->len * 10 + added_len;
		//printf("new key %d\n",req->key_info->len);
			if ( flag ) req->key_info->valid++;
			break;
		case 4:	// value
			req->value_info->len = req->value_info->len * 10 + added_len;
			if ( flag ) req->value_info->valid++;
			break;
	}
	if ( flag ) {
//		printf("GetLength2 :%c, %d, %d\n",cmp,which,length);
		return cur+1;
	}
	else return -1;
}

/*
   if type == 0 -> not yet
   else succ

   return -1 -> not yet
   else next postion to process
 */
int GetType(char *str, int len, int cur, req_t *req) {
	int left = req->type_info->len - req->type_info->offset;
	if ( left == 0 ) {
		if ( cur == len ) return -1;	// end of str
		else {
			if ( str[cur] == '\r') cur++;
			if ( cur < len && str[cur] == '\n' ) { 	// str[cur] == '\n'
				if ( strncmp(req->type_info->type_str, "SET", 4) == 0 ) req->type_info->type = 1;
				else if ( strncmp(req->type_info->type_str, "GET", 4) == 0 ) req->type_info->type = 2;
				else if ( strncmp(req->type_info->type_str, "DEL", 4) == 0 ) req->type_info->type = 3;
				req->type_info->valid++;
				//printf("Get Type1: %d\n",req->type_info->type);
				return cur+1;
			}
			else return -1;
		}
	}
	else {
		int available = (( len - cur > left ) ? left : len - cur) ;
		memcpy(&req->type_info->type_str[req->type_info->offset], &str[cur], available);
		req->type_info->offset += available;
		cur += available;

		/*
		   while ( cur < len && str[cur] != '\r' ) {	// copy str to type_str
		   type_str[req->type_info->offset++] = str[cur++];
		   }
		 */

		if ( cur == len ) {	// not completed
			return -1;
		}
		else {
			if ( str[cur] == '\r') cur++;
			if ( cur < len && str[cur] == '\n' ) {
				if ( strncmp(req->type_info->type_str, "SET", 4) == 0 ) req->type_info->type = 1;
				else if ( strncmp(req->type_info->type_str, "GET", 4) == 0 ) req->type_info->type = 2;
				else if ( strncmp(req->type_info->type_str, "DEL", 4) == 0 ) req->type_info->type = 3;	
				req->type_info->valid++;
				//printf("Get Type2: %d\n",req->type_info->type);
				return cur+1;
			}
			else return -1;
		}
	}
}

int GetKey(char *str, int len, int cur, req_t *req ) {
	int left = req->key_info->len - req->key_info->offset;
	if ( left == 0 ) {
		//printf("get 1\n");
		if ( cur == len ) return -1;
		else {
		//printf("get 2\n");
			if ( str[cur] == '\r' ) cur++;
			if ( cur < len && str[cur] == '\n' ) {
		//printf("get 3\n");
				//printf("Get Key1: %"PRIu64"\n",req->key_info->key);
				req->key_info->valid++;
				return cur+1;
			}
			else return -1;			
		}
	}
	else {
		//printf("get 4\n");
		int available = (( len - cur > left ) ? left : len - cur) ;
		int a = available;
		req->key_info->offset += available;
		while ( available-- ) { 
			if ( !isdigit(str[cur]) ) {
				cur++;
				continue;
			}
			req->key_info->key = req->key_info->key * 10 + str[cur++] - '0';
		}

		if ( cur == len ) {
		//printf("get 5\n");
		//printf("str : %s\nlen : %d\ncur : %d\nkeylen : %d\nkeyoffset : %d\navail : %d\n",str,len,cur,req->key_info->len, req->key_info->len - left, a);
			return -1;
		}
		else {
		//printf("get 6\n");
			if ( str[cur] == '\r' ) cur++;
			if ( cur < len && str[cur] == '\n' ) {
		//printf("get 7\n");
				req->key_info->valid++;
				//printf("Get Key2: %"PRIu64"\n",req->key_info->key);
				return cur+1;
			}
			else return -1;			
		}

	}
}

int GetValue(char* str, int len, int cur, req_t *req) {
	int left = req->value_info->len - req->value_info->offset;
	if ( left == 0 ) {
		if ( cur == len ) return -1;
		else {
			if ( str[cur] == '\r' ) cur++;
			if ( cur < len && str[cur] == '\n' ) {
				req->value_info->valid++;
				//printf("Get Value1: %s\n",req->value_info->value);
				return cur+1;
			}
			else return -1;			
		}
	}
	else {
		int available = (( len - cur > left ) ? left : len - cur) ;
		memcpy(&req->value_info->value[req->value_info->offset], &str[cur], available);
		req->value_info->offset += available;
		cur += available;

		if ( cur == len ) {
			return -1;
		}
		else {
			if ( str[cur] == '\r' ) cur++;
			if ( cur < len && str[cur] == '\n' ) {
				req->value_info->valid++;
				//printf("Get Value1: %s\n",req->value_info->value);
				return cur+1;
			}
			else return -1;			
		}

	}
}

int GetRequest(int fd, char *str, int len, req_t *req, unsigned int *start, unsigned int *end, heap_t *pq, pthread_mutex_t *mutx) {
	int cur = 0, result = 0;
	uint64_t seq = 0;
	req_t* new_req = req;
	//printf("cur %d, req %p\n", cur, req);
	while ( cur < len ) {
		printf("cur : %d\n", cur);
		if ( new_req == NULL ) {
			new_req = alloc_req(new_req);
			new_req->fd = fd;
		}
		else {
			//printf("keyword valid: %d\ntype valid: %d\nkey valid: %d\nvalue valid: %d\n",req->keyword_info->valid,req->type_info->valid,req->key_info->valid,req->value_info->valid);
		}

		if ( !new_req->keyword_info->valid )
			if ( (cur = GetLength(str, '*', len, cur, new_req, 1)) == -1 ) return -1;
//printf("1 %d\n",req->keyword_info->keywordNum); 
		if ( !new_req->type_info->valid ) 
			if ( (cur = GetLength(str, '$', len, cur, new_req, 2)) == -1 ) return -1;

//printf("2 %d\n",req->type_info->len); 
		if ( new_req->type_info->valid == 1 )
			if ( (cur = GetType(str, len, cur, new_req)) == -1 ) return -1;

//printf("3\n"); 
		if ( !new_req->key_info->valid ) 
			if ( (cur = GetLength(str, '$', len, cur, new_req, 3)) == -1 ) return -1;

//printf("4 %d\n",req->key_info->len); 
		if ( new_req->key_info->valid == 1 )
			if ( (cur = GetKey(str, len, cur, new_req)) == -1 ) return -1;

//printf("5\n"); 

		#ifdef MEMIO
		if ( new_req->type_info->type < 3 ) {
			alloc_dma(new_req);
		}	
		#endif


		if ( new_req->type_info->type > 1 ) {
			if ( new_req->type_info->type == 2 ) {
				new_req->seq = *end;
				new_req->pq = pq;
				new_req->cur = start;
				new_req->mutx = mutx;
				*end = (*end + 1) % 0xffffffff;
			}
			EnqueReq(new_req, seq);
			//make_req(req);
			new_req = NULL;
			continue;
		}

//printf("6\n"); 
		if ( !new_req->value_info->valid )
			if ( (cur = GetLength(str, '$', len, cur, new_req, 4)) == -1 ) return -1;

//printf("7\n"); 
		if ( new_req->value_info->valid == 1 ) 
			if ( (cur = GetValue(str, len, cur, new_req)) == -1 ) return -1;

//printf("8\n"); 
		EnqueReq(new_req, seq);
		new_req = NULL;
		//make_req(req);
	}
	return 0;
}

/*
int ParseAndInsertCommand(int fd, char *str, int len, uint64_t seq, req_t *req) {
	int str_len = len;
	int i = 0, j;
	int keyword_num = 0;
	int keyword_len;
	int start_keyword = 0;
	int cur = 0, cur_key;

	while ( i < str_len && str[i] == '*' ) { // command starting point
		cur = i;
		req = (req_t*)malloc(sizeof(req_t));
		req->value = (char*)malloc(sizeof(char) * PAGESIZE);
		memset(req->value, 0, PAGESIZE);
		req->fd = fd;
		while ( i < str_len && str[i++] != '\n');

		if( i >= str_len ) {	// command is not complete
			return cur;
		}
		else {			// number of keywords
			str[i-2] = 0;
			keyword_num = atoi(&str[cur+1]);

			for ( j=0; j < keyword_num; j++) {
				cur_key = i;	// str[cur] == str[i] == '$'
				if ( i < str_len && str[i++] == '$' ) {
					while ( i < str_len && str[i++] != '\n' );

					if ( i >= str_len ) {
						return cur;
					}
					else {
						str[i-2] = 0;
						keyword_len = atoi(&str[cur_key+1]);

						if ( i + keyword_len + 1 >= str_len ) {
							return cur;
						}

						str[i + keyword_len] = 0;
						if ( j == 0 ) {
							if ( strncmp(&str[i],"SET",4) == 0 ) req->type = 1;
							else if ( strncmp(&str[i],"GET",4) == 0 ) req->type = 2;
							else if ( strncmp(&str[i], "DEL",4) == 0 ) req->type = 3;
						}
						else if ( j == 1 ) {
							req->key = strtoull(&str[i+4],NULL,10);
							//printf("Parse Key: %"PRIu64"\n",req->key);
						}
						else if ( j == 2 ) {
							strncpy(req->value, &str[i], keyword_len);
							req->len = keyword_len;
						}
						i += keyword_len + 2;
					}
				}
				else {
					if ( i < str_len ) return -1;
					return cur;
				}
			}
		}
		
		   enqueue request
		 
		EnqueReq(req, seq);
	}
	if ( i == str_len ) return -2;	// succ!!!!
	else if ( i > str_len ) return cur;
	else return -1;
}
*/

int SendOkCommand(int fd) {
//	printf("socket %d\n",fd);
	if ( write(fd, "+OK\r\n", 5) == -1 ) {
		printf("send OK failed\n");
		return -1;
	}
	return 0;
}

int SendErrCommand(int fd, char* ErrMsg) {
	if ( write(fd, "-ERR ", 5) == -1 ) {
		printf("send Err failed\n");
		return -1;
	}
	if ( write(fd, ErrMsg, strlen(ErrMsg)) == -1 ) {
		printf("send Err failed\n");
		return -1;
	}
	if ( write(fd, "\r\n", 2) == -1 ) {
		printf("send Err failed\n");
		return -1;
	}
	return 0;
}

int SendBulkValue(int fd, char* value, int len) {
	char header[32];

	if ( len == -1 ) {
		if ( write(fd, "$-1\r\n", 5) == -1 ) {
			printf("send bulk value failed\n");
			return -1;
		}
		return 0;
	}
	sprintf(header, "$%d\r\n", len);

	if ( write(fd, header, strlen(header)) == -1 ) {
		printf("send bulk value failed\n");
		return -1;
	}
	if ( write(fd, value,len) == -1 ) {
		printf("send bulk value failed\n");
		return -1;
	}
	if ( write(fd, "\r\n", 2) == -1 ) {
		printf("send bulk value failed\n");
		return -1;
	}
	return 0;
}

int SendDelCommand(int fd) {
	if ( write(fd, ":1\r\n", 4) == -1 ) {
		printf("send Del failed\n");
		return -1;
	}
	return 0;
}
