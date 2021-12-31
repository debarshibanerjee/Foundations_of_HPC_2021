#!/bin/bash

echo "Cleaning old files..."
rm -f ring.log 2>/dev/null
rm -f final.csv 2>/dev/null

touch final.csv
touch ring.log

echo "Loading Open-MPI modules..."
module purge 2>/dev/null
module load openmpi-4.1.1+gnu-9.3.0 2>/dev/null
module list

for i in {2..24..1}; do
	mpic++ ring.cc -o ring.x 2>ring_$i.log
	echo "Running on $i processors..."
	out=$(mpirun -np $i ring.x 2>>ring_$i.log)
	mv ring.dat ring_$i.dat
	rm -f ring.dat 2>/dev/null
	rm -f ring-time.dat 2>/dev/null
	echo $i, $out >> final.csv
	echo "Log output when number of processors = $i" >> ring.log
	cat ring_$i.log >> ring.log
	echo " " >> ring.log
	rm -f ring_$i.log 2>/dev/null
done

echo "DONE"
