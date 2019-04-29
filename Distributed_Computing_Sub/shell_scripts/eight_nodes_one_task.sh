#!/bin/bash -l

#SBATCH --mem-per-cpu=3G
#SBATCH --time=18:00:00
#SBATCH --partition=killable.q
#SBATCH --constraint=elves
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=OpenMPI_FindLines

module load OpenMPI
mpirun $HOME/CurrentWork/OpenMPI_FindLines
