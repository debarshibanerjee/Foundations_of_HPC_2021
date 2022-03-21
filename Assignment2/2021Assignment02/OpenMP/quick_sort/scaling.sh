#!/bin/bash

#PBS -l nodes=1:ppn=48
#PBS -l walltime=3:00:00
#PBS -q dssc_gpu
#PBS -m abe
#PBS -M debarshibanerjee@gmail.com

cd "$PBS_O_WORKDIR" || exit

echo "Loading modules..."
module load gnu

echo "Cleaning old files..."
make clean
rm -rf *.dat data/ strong_omp_scaling.png weak_omp_scaling.png
mkdir -p data

echo "Compiling..."
make

#10^8
N=100000000
for i in {0..5..1};do
	num_threads=$((2**i))
	for j in {1..3..1};do
		echo "Run $j with $num_threads threads and building a kd-tree with $N points..."
		export OMP_NUM_THREADS=$num_threads
		./kdtree_omp.x $N >> data/strong_omp_$num_threads.dat
	done
done

#10^7 * num_threads(1-32)
for i in {0..5..1};do
	num_threads=$((2**i))
	N=$((10000000*num_threads))
	for j in {1..3..1};do
		echo "Run $j with $num_threads threads and building a kd-tree with $N points..."
		export OMP_NUM_THREADS=$num_threads
		./kdtree_omp.x $N >> data/weak_omp_$num_threads.dat
	done
done

for file in ./data/strong*.dat;do
	awk '{x+=$1;y+=$2} END{printf "%.0f     %.6f\n",x/NR,y/NR}' $file 1>> strong_omp_scaling.dat
done

for file in ./data/weak*.dat;do
	awk '{x+=$1;y+=$2} END{printf "%.0f     %.6f\n",x/NR,y/NR}' $file 1>> weak_omp_scaling.dat
done

cat strong_omp_scaling.dat | sort -n > strong_tmp.dat
cat weak_omp_scaling.dat | sort -n > weak_tmp.dat

mv strong_tmp.dat strong_omp_scaling.dat
mv weak_tmp.dat weak_omp_scaling.dat

echo "To plot, run the 'plot_strong.gp' and 'plot_weak.gp' scripts with gnuplot"
echo "USAGE: gnuplot plot_strong.gp"
echo "This will automatically generate the relevant strong and weak scaling graphs"
