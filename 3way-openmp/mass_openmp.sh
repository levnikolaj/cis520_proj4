#!/bin/bash

sbatch --constraint=elves --time=0:10:00 --mem-per-cpu=5G --nodes=1 --cpus-per-task=2 shell_script_openmp.sh
