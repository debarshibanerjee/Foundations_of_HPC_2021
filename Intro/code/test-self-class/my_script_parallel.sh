#!/bin/bash

#PBS -l nodes=1:ppn=1
#PBS -l walltime=5:00:00
#PBS -q dssc

cd $PBS_O_WORKDIR
./pi.x 1000000000

# parallel execution
module load openmpi/4.0.3/gnu
mpirun -np 4 ./mpi_pi.x 1000000000

# if we run it now, then we are asking it to do the calculation for EACH process, so 4x100..(as above)
# so it will take same time as serial execution if we run it now.

exit
