#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "Illegal number of parameters"
	echo "Run this script with 1 argument - number of processors"
	echo "For example, './ring.sh 4' to use 4 processors"
	exit 11
fi

smt_enabled=$(cat /sys/devices/system/cpu/smt/active)
num_proc=$(cat /proc/cpuinfo | grep processor | wc -l)
if [ "$smt_enabled" -eq 1 ]; then
	num_proc=$((num_proc/2))
fi

echo "Cleaning old files..."
rm -f ring.dat 2>/dev/null

echo "Compiling..."
mpic++ ring.cc -o ring.x

if [ "$1" -le 0 ]; then
	echo "Number of processors cannot be 0 or negative"
	exit 33
fi

echo " " && echo "Running on $1 processors..."

if [ "$1" -le "$num_proc" ]; then
	mpirun -np $1 ring.x
else
	echo "WARNING: You are running in --oversubscribe mode"
	mpirun -np $1 --oversubscribe ring.x
fi

echo " " && echo "Printing ring.dat on screen..."
cat ring.dat
