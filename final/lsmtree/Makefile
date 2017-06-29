EXTRA_CFLAGS= -Wno-maybe-uninitialized
EXTRA_CFLAGS+= -march=armv7-a -mtune=cortex-a9 -mfpu=neon
EXTRA_LDFLAGS= -march=armv7-a -mtune=cortex-a9 -mfpu=neon -Wl,--fix-cortex-a8
HOST=arm-linux-gnueabihf
CC=${HOST}-g++ -D_FILE_OFFSET_BITS=64
CXX=${HOST}-g++ -D_FILE_OFFSET_BITS=64
LD=${HOST}-ld
#CC=g++ 
#CXX=g++ 
#LD=ld

LIBFTL_LIB= ../bdbm_drv/frontend/libmemio/libmemio.a
AM_LINK= $(AM_V_CCLD)$(CXX) $^ $(EXEC_LDFLAGS) -o $@ $(LDFLAGS) $(COVERAGEFLAGS) $(LIBFTL_LIB)

LIBFTL_INC += \
      -I../bdbm_drv/frontend/libmemio \
      -I../bdbm_drv/frontend/nvme \
      -I../bdbm_drv/ftl \
      -I../bdbm_drv/include \
      -I../bdbm_drv/common/utils \
      -I../bdbm_drv/common/3rd \
      -I../bdbm_drv/devices/common \
      -I./ \
      -D USER_MODE \
      -D HASH_BLOOM=20 \
      -D CONFIG_ENABLE_MSG \
      -D CONFIG_ENABLE_DEBUG \
      -D USE_PMU \
      -D USE_NEW_RMW \
      -D ZYNQ=1 \

CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)
CFLAGS += $(LIBFTL_INC) 
CXXFLGAS = $(CFLAGS)
CXXFLAGS += $(LIBFTL_INC) 

CXXFLAGS += -std=c++11 -D_DEFAULT_SOURCE
CXXFLAGS += -DNOHOST -DENABLE_LIBFTL
CFLAGS += -std=c++11 -D_DEFAULT_SOURCE
CFLAGS += -DNOHOST -DENABLE_LIBFTL
LDFLAGS += -L. -lmemio -lpthread

OBJECTS = lsm_main.o bloomfilter.o bptree.o threading.o queue.o heap.o measure.o skiplist.o lsmtree.o

lsm : lsm_n   lsm_p

lsm_n : $(OBJECTS) libmemio.a
	$(CXX) -o exefile/lsm_n.o $(OBJECTS) $(LDFLAGS)

lsm_p : $(OBJECTS) libmemio.a
	$(CXX) -o exefile/lsm_p.o $(OBJECTS) -D NPRINTOPTION $(LDFLAGS)

lsm_main.o :  

bptree.o : skiplist.c bloomfilter.c utils.c queue.c heap.c

bloomfilter.o : utils.h

threading.o : $*.*

utils.o : $*.

queue.o : $*.*

heap.p : $*.*

skiplist.o : $*.*

clean :
	rm -f *.o

