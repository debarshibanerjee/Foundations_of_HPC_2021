#!/bin/bash

echo "Cleaning old files..."
make clean

echo "If you want verbose output, type 'verbose', else type anything else and press enter"
read -r verbose_flag

echo "If you want to print the kd tree in a tree.dot file, type 'print', else type anything else and press enter"
read -r print_tree_flag

echo "If you want to have debug information, type 'debug', else type anything else and press enter"
read -r debug_flag

echo "If you want to have quicksort used instead of quickselect, type 'qsort', else type anything else and press enter"
read -r qsort_flag

cflag_param=" "

if [ "$verbose_flag" = "verbose" ]; then
	cflag_param=$cflag_param"-D VERBOSE"
fi

if [ "$print_tree_flag" = "print" ]; then
	cflag_param=$cflag_param" -D PRINT_TREE"
fi

if [ "$debug_flag" = "debug" ]; then
	cflag_param=$cflag_param" -D DEBUG"
fi

if [ "$qsort_flag" = "qsort" ]; then
	cflag_param=$cflag_param" -D QUICK_SORT"
fi

make CFLAGS="$cflag_param"

echo "Compilation Done"

echo "Enter N (size of dataset)"
read -r N

echo "Enter number of OpenMP threads to be used"
read -r num_threads

echo "Running with $num_threads threads and building a kd-tree with $N points..."
export OMP_NUM_THREADS=$num_threads
./kdtree_omp.x $N

if [ "$print_tree_flag" = "print" ]; then
	echo "Generating binary tree image from tree.dot"
	dot -T png -O tree.gv
fi
