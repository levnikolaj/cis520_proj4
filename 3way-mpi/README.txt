How to run mpi on beocat?
Make sure the files have UNIX line terminators (Notepad++: Edit -> EOL Converstion -> Unix/LF)
Make sure to have execute access on shell scripts (chmod u+x filename, some variation of command)

1. Load proper module

module load OpenMPI

2. Compile .c file

mpicc -o mpi_comp mpi.c

3. Have shell scripts OR type out sbatch command (need '#!/bin/bash' at top of shell scripts)

./mass_mpi.sh -> sbatch every shell script
    sbatch ./shell1.sh
    sbatch ./shell2.sh
    ...

Each shell script (ex: shell1.sh) contains something like this...
Change '--nodes' & '--ntasks-per-node' to change MPI specifications

#!/bin/bash -l

#SBATCH --mem-per-cpu=3G
#SBATCH --time=1:00:00
#SBATCH --partition=killable.q
#SBATCH --constraint=elves
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=8
#SBATCH --job-name=3way-mpi

module load OpenMPI
mpirun $HOME/Proj4_520/3way-mpi/mpi_comp
