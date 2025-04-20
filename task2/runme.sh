#!/bin/bash

make clean
make

count=10
pids=()
for (( i=0; i<$count; i++ )); do
	./main my_file.txt &
	pids+=($!)
done

sleep 5m

for pid in ${pids[@]}; do
	kill -SIGINT $pid
done