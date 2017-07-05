#CFLAGS= -Wno-maybe-uninitialized
#EXTRA_CFLAGS+= -march=armv7-a -mtune=cortex-a9 -mfpu=neon
#EXTRA_LDFLAGS= -march=armv7-a -mtune=cortex-a9 -mfpu=neon -Wl,--fix-cortex-a8
HOST=arm-linux-gnueabihf
CC=${HOST}-g++ -D_FILE_OFFSET_BITS=64
CXX=${HOST}-g++ -D_FILE_OFFSET_BITS=64
LD=${HOST}-ld

#CC = g++ -D_FILE_OFFSET_BITS=64
#CXX = g++ -D_FILE_OFFSET_BITS=64
#LDFLAGS += -lpthread
#CXXFLAGS += \
         -I ./ \
         -g \
         -std=c++11 \
         -D NOHOST \
         -D NPRINTOPTION \
        
#CFLAGS += $(CXXFLAGS)
 
LIBFTL_LIB= ../../bdbm_drv/frontend/libmemio/libmemio.a
AM_LINK= $(AM_V_CCLD)$(CXX) $^ $(EXEC_LDFLAGS) -o $@ $(LDFLAGS) $(COVERAGEFLAGS) $(LIBFTL_LIB)
 
LIBFTL_INC += \
       -I../../bdbm_drv/frontend/libmemio \
       -I../../bdbm_drv/frontend/nvme \
       -I../../bdbm_drv/ftl \
       -I../../bdbm_drv/include \
       -I../../bdbm_drv/common/utils \
       -I../../bdbm_drv/common/3rd \
       -I../../bdbm_drv/devices/common \
       -I./ \
       -D USER_MODE \
       -D HASH_BLOOM=20 \
       -D CONFIG_ENABLE_MSG \
       -D CONFIG_ENABLE_DEBUG \
       -D USE_PMU \
       -D USE_NEW_RMW \
       #-D ZYNQ=1 \
 
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)
CFLAGS += $(LIBFTL_INC)
CXXFLAGS = $(CFLAGS)
CXXFLAGS += $(LIBFTL_INC)

CXXFLAGS += -std=c++11 -D_DEFAULT_SOURCE -D NPRINTOPTION
CXXFLAGS += -DNOHOST -DENABLE_LIBFTL -D NPRINTOPTION
CFLAGS += -std=c++11 -D_DEFAULT_SOURCE
CFLAGS += -DNOHOST -DENABLE_LIBFTL
#CFLAGS += -DNOHOST 
LDFLAGS += -L./ -lmemio -lpthread
#LDFLAGS += -lpthread 


OBJECTS = server.o command.o queue.o request.o priority_queue.o lsmtree.o bptree.o utils.o measure.o skiplist.o threading.o LR_inter.o

#OBJECTS = server.c command.c queue.c request.c priority_queue.c lsmtree.c bptree.c utils.c measure.c skiplist.c threading.c LR_inter.c

test : $(OBJECTS)
	$(CXX) -o test $(OBJECTS) $(LDFLAGS)

priority_queue.o : $*.*
        
server.o : $*.*

command.o : $*.*

queue.o : $*.*

requestt.o : $*.* priority_queue.c

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
