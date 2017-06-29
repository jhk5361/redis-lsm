#ifndef __COMMAND_H__
#define __COMMAND_H_
#include <stdint.h>
//#include "priority_queue.h"
#include <pthread.h>
#include "request_lsm.h"

int GetLength(char*, char, int, int, req_t, char);
int GetType(char*, int, int, req_t);
int GetKey(char*, int, int, req_t);
int GetValue(char*, int, int, req_t);
req_t* AllocRequest(req_t*);
int GetRequest(int, char*, int, req_t*, unsigned int*, unsigned int*, pthread_mutex_t*);
//int ParseAndInsertCommand(int, char*, int, uint64_t);
int SendOkCommand(int);
int SendErrCommand(int, char*);
int SendBulkValue(int, char*, int);
int SendDelCommand(int);

#endif
