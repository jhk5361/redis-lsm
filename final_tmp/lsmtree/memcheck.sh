#! /bin/bash
rm log/log_file_m
rm err/err_file_m

$(valgrind --leak-check=full --show-leak-kinds=all --tool=memcheck $1 >> log/log_file_m 2>>err/err_file_m)

