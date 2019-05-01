How to run openmp on beocat?
Make sure the files have UNIX line terminators (Notepad++: Edit -> EOL Converstion -> Unix/LF)
Make sure to have execute access on shell scripts (chmod u+x filename, some variation of command)

1. Create object file

gcc -o openmp_comp -fopenmp openmp.c

2. Have shell scripts OR type out sbatch command (need ''#!/bin/bash' at top of shell scripts)

./mass_openmp.sh -> contains sbatch command
OR
sbatch --constraint=elves --time=0:10:00 --mem-per-cpu=5G --nodes=1 --cpus-per-task=2 shell_script_openmp.sh

shell_script_openmp.sh -> /homes/levnikolaj/Proj4_520/3way-openmp/openmp_comp
