#!/bin/bash

#PBS -l nodes=1:ppn=1
#PBS -l walltime=5:00:00
#PBS -q dssc

cd $PBS_O_WORKDIR
./pi.x 1000000000

exit
