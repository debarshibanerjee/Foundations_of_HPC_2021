#!/bin/bash

#PBS -l nodes=2:ppn=24
#PBS -l walltime=1:00:00
#PBS -q dssc_gpu

cd "$PBS_O_WORKDIR" || exit

echo "" && echo "Removing old files..."
rm -f final-gpu.csv 2>/dev/null
rm -rf data_gpu 2>/dev/null
mkdir -p data_gpu/run_data
mkdir -p data_gpu/task_data
touch final-gpu.csv

echo "" && echo "Loading OpenMPI..."
module purge 2>/dev/null
module load openmpi-4.1.1+gnu-9.3.0 2>/dev/null
module list

echo "" && echo "Compiling..."
rm -f jacobi3D_gpu.x 2>/dev/null
mpif77 -ffixed-line-length-none Jacobi_MPI_vectormode.F -o jacobi3D_gpu.x

# map_by $1; np $2;
function jacobi {
	mpirun --map-by $1 --mca btl ^openib -np $2 ./jacobi3D_gpu.x <input.1200 2>/dev/null 1>jacobi-$1-$2-gpu.dat
	awk 'BEGIN{i=0} {if($1==4){x+=$9;y+=$16;i+=1} if($4==2){z=$10}} END{printf "%.6f, %.6f, %.2f, ",x/i,y/i,z;print FILENAME}' jacobi-$1-$2-gpu.dat 1>>final-gpu.csv
	mv jacobi-$1-$2-gpu.dat data_gpu/run_data
	mkdir -p data_gpu/task_data/task_$1_$2
	mv *.dat data_gpu/task_data/task_$1_$2
}

system=GPU

for map_by in core socket; do
	for i in {0..12..4}; do
		if [ $i -eq 0 ]; then
			i=1 # serial condition
		fi
		echo "Running map = $map_by, nproc = $i, on $system nodes"
		jacobi $map_by $i
	done
done

map_by=node
for i in {12..48..12};do
	echo "Running map = $map_by, nproc = $i, on $system nodes"
	jacobi $map_by $i
done

echo "DONE"
