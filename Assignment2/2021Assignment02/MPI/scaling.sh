#!/bin/bash

echo "Loading modules..."
module load gnu
module load openmpi

echo "Cleaning old files..."
make clean
rm -rf *.dat data/ strong_mpi_scaling.png weak_mpi_scaling.png
mkdir -p data

echo "Compiling..."
make

#10^8
N=100000000
for i in {0..5..1};do
	num_proc=$((2**i))
	for j in {1..10..1};do
		echo "Run $j with $num_proc processes and building a kd-tree with $N points..."
		mpirun -np $num_proc --oversubscribe ./kdtree_mpi.x $N >> data/strong_mpi_$num_proc.dat
	done
done

#10^7 * num_proc(1-32)
for i in {0..5..1};do
	num_proc=$((2**i))
	N=$((10000000*num_proc))
	for j in {1..10..1};do
		echo "Run $j with $num_proc processes and building a kd-tree with $N points..."
		mpirun -np $num_proc --oversubscribe ./kdtree_mpi.x $N >> data/weak_mpi_$num_proc.dat
	done
done

for file in ./data/strong*.dat;do
	awk '{x+=$1;y+=$2} END{printf "%.0f     %.6f\n",x/NR,y/NR}' $file 1>> strong_mpi_scaling.dat
done

for file in ./data/weak*.dat;do
	awk '{x+=$1;y+=$2} END{printf "%.0f     %.6f\n",x/NR,y/NR}' $file 1>> weak_mpi_scaling.dat
done

cat strong_mpi_scaling.dat | sort -n > strong_tmp.dat
cat weak_mpi_scaling.dat | sort -n > weak_tmp.dat

mv strong_tmp.dat strong_mpi_scaling.dat
mv weak_tmp.dat weak_mpi_scaling.dat

echo "To plot, run the 'plot_strong.gp' and 'plot_weak.gp' scripts with gnuplot"
echo "USAGE: gnuplot plot_strong.gp"
echo "This will automatically generate the relevant strong and weak scaling graphs"
