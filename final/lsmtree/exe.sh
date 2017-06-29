#!/bin/bash

if [ "$1" == "s" ]
then
	make lsm
	./exefile/lsm_n.o > log/lsm_log_skip.log
	echo "skip done"
elif [ "$1" == "pr" ]
then
	make rand
	make lsm
	./exefile/rand.o	
	./exefile/lsm_n.o > log/lsm_log_skip_pr.log
	echo "skip done"
elif [ "$1" == "gr" ]
then
	make lsm
	./exefile/lsm_n.o > log/lsm_log_skip_gr.log
	echo "skip done"
elif [ "$1" == "r" ]
then
	make rand
	make lsm
	./exefile/rand.o
	./exefile/lsm_n.o > log/lsm_log_skip_r.log
	echo "skip done"
fi
