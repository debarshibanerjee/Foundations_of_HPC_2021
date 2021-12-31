#!/bin/bash

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
filename='matrix_second.in'

while read -r line; do
	counter=$((counter+1))
	sum=0
	mpic++ sum3Dmatrix.cc -o sum3Dmatrix.x 2>matrix_$counter.log
	echo "For topology (size1 size2 size3 ndims dim1 dim2 dim3) - $line"
	for j in {1..30..1}; do
		out=$(mpirun -np 16 ./sum3Dmatrix.x $line </dev/null 2>>matrix_$counter.log)
		# </dev/null (redirecting /dev/null to stdin is used to avoid having the script wait for input
		sum=$(echo "$sum + $out" | bc -l)
	done
	avg=$(echo "$sum / $j" | bc -l)
	echo "$line	- $avg" >> final.dat
	echo "Log output when topology = $line" >> matrix.log
	cat matrix_$counter.log >> matrix.log
	echo " " >> matrix.log
	rm -f matrix_$counter.log 2>/dev/null
done < "$filename"

echo "DONE"
