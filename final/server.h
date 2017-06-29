#ifndef __SERVER_H__
#define __SERVER_H__

#define BUF_SIZE 8500
#define MAX_CLNT 256

int init_server(int);
int execute_server(int);
void* work_on_accept(void*);

#endif
