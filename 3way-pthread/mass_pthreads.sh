#!/bin/bash

for i in 1 2 4 8 16
do
sbatch --constraint=elves --time=3:00:00 --mem-per-cpu=3G --nodes=1 --cpus-per-task=$i pthreads_script.sh $i
done
