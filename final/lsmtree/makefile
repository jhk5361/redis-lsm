All: archive
lsm : lsm_n lsm_p

TOP_DIR=./

LR: lsm_main.c lsmtree.c bptree.c utils.c measure.c skiplist.c threading.c LR_inter.c queue.c
	gcc -g -o exefile/LR $^ -std=c99 -I$(TOP_DIR)/src -I$(TOP_DIR)/lsmtree -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_BSD_SOURCE -lpthread


PLR: lsm_main.c lsmtree.c bptree.c utils.c measure.c skiplist.c threading.c LR_inter.c queue.c
	gcc -g -o exefile/PLR $^ -std=c99 -I$(TOP_DIR)/src -I$(TOP_DIR)/lsmtree -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_BSD_SOURCE -DNPRINTOPTION -lpthread

lsm_n: lsm_main.c lsmtree.c bloomfilter.c bptree.c utils.c queue.c utils.h heap.c measure.c skiplist.c threading.c#stack.c
	gcc -g -o exefile/lsm_n.o $^ -std=c99 -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -lpthread

lsm_p : lsm_main.c lsmtree.c bloomfilter.c bptree.c utils.c queue.c utils.h heap.c measure.c skiplist.c threading.c#stack.c
	gcc -g -o exefile/lsm_p.o $^ -std=c99 -D NPRINTOPTION -D_DEFAULT_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -lpthread

lsm_hn: lsm_main.c lsmtreeh.c bloomfilter.c bptree.c hash.c utils.c queue.c utils.h heap.c measure.c skiplist.c#stack.c
	gcc -pg -g -o exefile/lsm_h.o $^ -std=c99 -D_DEFAULT_SOURCE

skip: skiplist.*
	gcc -g -o exefile/skip.o skiplist.c -D DEBUG

bptree: skiplist.c bptree.c bloomfilter.c utils.c queue.c heap.c#stack.c
	gcc -g -o exefile/bptree.o $^ -D DEBUG_B

utils: utils.*
	gcc -g -o uexefile/tils.o $^ -D DEBUG

stack: stack.*
	gcc -g -o exefile/stack.o $^ -D DEBUG -std=c99

queue: queue.*
	gcc -g -o exefile/queue.o $^ -D DEBUG -std=c99

test: test.c utils.h
	gcc -g -o exefile/test.o test.c -Wall -std=c99

rand : rand.c utils.h
	gcc -g -o exefile/rand.o rand.c -std=c99

heap : heap.*
	gcc -g -o exefile/heap.o heap.c -D DEBUG -std=c99
hash : hash.*
	gcc -g -o exefile/hash.o $^ -D DEBUG -std=c99

bloomfilter : bloomfilter.* utils.h
	gcc -g -o bloomfilter.o bloomfilter.* -D DEBUG -std=c99
