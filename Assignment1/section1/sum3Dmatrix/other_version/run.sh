#!/bin/bash

#PBS -l nodes=1:ppn=24
#PBS -l walltime=12:00:00
#PBS -q dssc

cd "$PBS_O_WORKDIR" || exit

echo "Cleaning old files..."
rm -f matrix.log 2>/dev/null
rm -f final.dat 2>/dev/null
touch final.dat
touch matrix.log

echo "Loading Open-MPI modules..."
module purge 2>/dev/null
module load openmpi-4.1.1+gnu-9.3.0 2>/dev/null
module list

counter=0
filename='matrix.in'

while read -r line; do
	counter=$((counter+1))
	mpic++ sum3Dmatrix.cc -o sum3Dmatrix.x 2>matrix_$counter.log
	echo "For topology (size1 size2 size3 ndims dim1 dim2 dim3) - $line"
	out=$(mpirun -np 24 ./sum3Dmatrix.x $line </dev/null 2>>matrix_$counter.log)
	echo "$line	- $out" >> final.dat
	echo "Log output when topology = $line" >> matrix.log
	cat matrix_$counter.log >> matrix.log
	echo " " >> matrix.log
	rm -f matrix_$counter.log 2>/dev/null
done < "$filename"

echo "DONE"
