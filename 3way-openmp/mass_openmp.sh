#!/bin/bash

sbatch --constraint=elves --time=1:00:00 --mem-per-cpu=1G --nodes=1 --cpus-per-task=12 shell_script_openmp.sh
