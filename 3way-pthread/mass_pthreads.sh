#!/bin/bash

sbatch --constraint=elves --time=0:10:00 --mem-per-cpu=5G --nodes=1 --cpus-per-task=3 shell_script_pthreads.sh
