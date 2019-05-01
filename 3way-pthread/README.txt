How to run pthreads on beocat?
Make sure the files have UNIX line terminators (Notepad++: Edit -> EOL Converstion -> Unix/LF)
Make sure to have execute access on shell scripts (chmod u+x filename, some variation of command)

1. Create object file

gcc -o pthreads_comp -pthread pthreads.c

2. Have shell scripts OR type out sbatch command (need ''#!/bin/bash' at top of shell scripts)

./mass_pthreads.sh -> contains sbatch command
OR
sbatch --constraint=elves --time=0:10:00 --mem-per-cpu=5G --nodes=1 --cpus-per-task=2 shell_script_pthreads.sh

shell_script_pthreads.sh -> /homes/levnikolaj/Proj4_520/3way-pthread/pthreads_comp
