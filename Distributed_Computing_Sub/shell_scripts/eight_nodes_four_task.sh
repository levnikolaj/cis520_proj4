#!/bin/bash -l

#SBATCH --mem-per-cpu=3G
#SBATCH --time=12:30:00
#SBATCH --partition=killable.q
#SBATCH --constraint=elves
#SBATCH --nodes=8
#SBATCH --ntasks-per-node=4
#SBATCH --job-name=OpenMPI_FindLines

module load OpenMPI
mpirun $HOME/CurrentWork/OpenMPI_FindLines
