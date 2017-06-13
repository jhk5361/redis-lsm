CC = gcc
CXX = g++
LDFLAGS += -lpthread
CFLAGS += -I ./

OBJECTS = server.o command.o queue.o request.o

test : $(OBJECTS)
	$(CC) -o test $(OBJECTS) $(LDFLAGS)

server.o : $*.* request.h

command.o : $*.*

queue.o : $*.*

request.o : $*.*

clean :
	rm -f *.o
