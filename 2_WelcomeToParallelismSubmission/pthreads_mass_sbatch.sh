#!/bin/bash

for i in 1 2 4 8 16
do
	sbatch --constraint=elves --time=0:05:00 --mem-per-cpu=512M --nodes=1 --cpus-per-task=$i execute_gcc_pthread_comp.sh $i
done 