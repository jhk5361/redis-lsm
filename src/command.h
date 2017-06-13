#ifndef __COMMAND_H__
#define __COMMAND_H_
#include <stdint.h>
#include "request.h"

int GetLength(char*, char, int, int, req_t, char);
int GetType(char*, int, int, req_t);
int GetKey(char*, int, int, req_t);
int GetValue(char*, int, int, req_t);
req_t* AllocRequest(req_t*);
req_t* GetRequest(int, char*, int, uint64_t, req_t*);
//int ParseAndInsertCommand(int, char*, int, uint64_t);
int SendOkCommand(int);
int SendErrCommand(int, char*);
int SendBulkValue(int, char*, int);
int SendDelCommand(int);

#endif
