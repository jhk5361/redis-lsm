CC = gcc
CXX = g++
LDFLAGS += -lpthread
CFLAGS += -I ./

OBJECTS = server.o command.o queue.o request.o priority_queue.o

test : $(OBJECTS)
	$(CC) -o test $(OBJECTS) $(LDFLAGS)

priority_queue.o : $*.*

server.o : $*.*

command.o : $*.*

queue.o : $*.*

request.o : $*.* priority_queue.c


clean :
	rm -f *.o
