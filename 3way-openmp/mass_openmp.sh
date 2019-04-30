#!/bin/bash

sbatch --constraint=elves --time=0:05:00 --mem-per-cpu=512M --nodes=1 --cpus-per-task=1 shell_script_openmp.sh 