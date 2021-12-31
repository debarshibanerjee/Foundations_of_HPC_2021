#!/bin/bash

#PBS -l nodes=2:ppn=48
#PBS -l walltime=1:00:00
#PBS -q dssc_gpu

cd "$PBS_O_WORKDIR" || exit

echo "Nodes used -"
nodes_used_1=$(cat $PBS_NODEFILE | uniq -d | sort | head -n 1)
nodes_used_2=$(cat $PBS_NODEFILE | uniq -d | sort | tail -n 1)

# GCC
echo "" && echo "Loading GCC..."
module purge 2>/dev/null
module load openmpi-4.1.1+gnu-9.3.0 2>/dev/null
module list

function openmpi {
	cmd_gnu="mpirun -np 2 --report-bindings --map-by $1 --mca pml $2 --mca btl self,$3 --mca btl_$3_if_include $4 ./IMB-MPI1_gcc PingPong -msglog 28"
	for i in {1..10..1}; do
		echo "$1, $2, $3, $4 --- Run - $i"
		mkdir -p output_gpu/openmpi_run_$i
		echo "# Command used - $cmd_gnu" > output_gpu/openmpi_run_$i/openmpi_$1_$2_$3_$4.csv
		echo "# Nodes used - $nodes_used_1, $nodes_used_2" >> output_gpu/openmpi_run_$i/openmpi_$1_$2_$3_$4.csv
		echo "# Calculated Latency = ; Calculated Bandwidth = " >> output_gpu/openmpi_run_$i/openmpi_$1_$2_$3_$4.csv
		$cmd_gnu 2>/dev/null | grep -v ^\# | grep -v '^$' | sed -r 's/  */,/g' | cut -d "," -f2- >> output_gpu/openmpi_run_$i/openmpi_$1_$2_$3_$4.csv
	done
}

for map_by in core socket node; do
	for pml in ob1 ucx; do
		for btl in tcp vader; do
			if [ $btl == vader ]
			then
				if [ $map_by == node ]
				then
					echo "For map-by node, shared memory=vader is NOT possible"
					continue
				fi
			fi
			for net in br0 ib0; do
				openmpi $map_by $pml $btl $net
			done
		done
	done
done

module purge 2>/dev/null
echo "" && echo "DONE - GCC"

# Intel
echo "" && echo "Loading Intel..."
module load intel 2>/dev/null
module list

function intelmpi {
	cmd_intel="mpiexec -np 2 -env I_MPI_DEBUG 5 -genv I_MPI_FABRICS=$1 -genv I_MPI_OFI_PROVIDER=$2 -genv I_MPI_PIN_PROCESSOR_LIST 0,$3 ./IMB-MPI1_intel PingPong -msglog 28"
	for i in {1..10..1}; do
		echo "$4, $1, $2 --- Run - $i"
		mkdir -p output_gpu/intelmpi_run_$i
		echo "# Command used - $cmd_intel" > output_gpu/intelmpi_run_$i/intelmpi_$4_$1_$2.csv
		echo "# Nodes used - $nodes_used_1, $nodes_used_2" >> output_gpu/intelmpi_run_$i/intelmpi_$4_$1_$2.csv
		echo "# Calculated Latency = ; Calculated Bandwidth = " >> output_gpu/intelmpi_run_$i/intelmpi_$4_$1_$2.csv
		$cmd_intel 2>/dev/null | grep -v ^\# | grep -v '^$' | grep -v '^\[' | sed -r 's/  */,/g' | cut -d "," -f2- >> output_gpu/intelmpi_run_$i/intelmpi_$4_$1_$2.csv
	done
}

for fabrics in shm ofi shm:ofi; do
	for ofi in tcp mlx shm sockets; do
		for pin in 1 2; do
			if [ $pin -eq 1 ]
			then
				map='contiguous'
			else
				map='socket'
			fi
			intelmpi $fabrics $ofi $pin $map $name_2
		done
	done
done

function intelmpi_node {
	cmd_intel_node="mpiexec -np 2 -ppn=1 -env I_MPI_DEBUG 5 -genv I_MPI_FABRICS=$1 -genv I_MPI_OFI_PROVIDER=$2 ./IMB-MPI1_intel PingPong -msglog 28"
	for i in {1..10..1}; do
		echo "node, $1, $2 --- Run - $i"
		mkdir -p output_gpu/intelmpi_run_$i
		echo "# Command used - $cmd_intel_node" > output_gpu/intelmpi_run_$i/intelmpi_node_$1_$2.csv
		echo "# Nodes used - $nodes_used_1, $nodes_used_2" >> output_gpu/intelmpi_run_$i/intelmpi_node_$1_$2.csv
		echo "# Calculated Latency = ; Calculated Bandwidth = " >> output_gpu/intelmpi_run_$i/intelmpi_node_$1_$2.csv
		$cmd_intel_node 2>/dev/null | grep -v ^\# | grep -v '^$' | grep -v '^\[' | sed -r 's/  */,/g' | cut -d "," -f2- | tail -n +3 >> output_gpu/intelmpi_run_$i/intelmpi_node_$1_$2.csv
	done
}

for fabrics in ofi shm:ofi; do
	for ofi in tcp mlx sockets; do
			intelmpi_node $fabrics $ofi
	done
done

rename : - output_gpu/*/*

module purge 2>/dev/null
echo "" && echo "DONE - Intel"

