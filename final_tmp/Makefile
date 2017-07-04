#CC = g
CC = g++ -D_FILE_OFFSET_BITS=64
CXX = g++ -D_FILE_OFFSET_BITS=64
LDFLAGS += -lpthread
CXXFLAGS += \
	-I ./ \
	-g \
	-std=c++11 \
	-D NOHOST \
	-D NPRINTOPTION \
	 
CFLAGS += $(CXXFLAGS)

OBJECTS = server.o command.o queue.o request.o priority_queue.o lsmtree.o bptree.o utils.o measure.o skiplist.o threading.o LR_inter.o

test : $(OBJECTS)
	$(CXX) -o test $(OBJECTS) $(LDFLAGS) $(CXXFLAGS) 

priority_queue.o : $*.*
	
server.o : $*.*

command.o : $*.*

queue.o : $*.*

request.o : $*.* priority_queue.c

skip.o : $*.*

bptree.o : $*.*

utils.o : $*.*

measure.o : $*.*

skiplist.o : $*.* measure.c

threading.o : $*.* measure.c

bptree.o : $*.* skiplist.c utils.c

lsmtree.o : $*.* bptree.c skiplist.c measure.c

LR_inter.o : $*.*

clean :
	rm -f *.o
