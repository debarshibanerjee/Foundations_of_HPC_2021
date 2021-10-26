/*
 * =====================================================================================
 *
 *       Filename:  first-mpi.c
 *
 *    Description:  First MPI program
 *
 *        Version:  1.0
 *        Created:  26/10/21 11:45:54 AM CEST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Debarshi Banerjee
 *   Organization:  ICTP, SISSA
 *
 * =====================================================================================
 */

// Run with:
// mpicc first-mpi.c -o first-mpi.x
// mpirun -np 4 first-mpi.x

// Alternatively to push the limits on laptops:
// mpirun --oversubscribe -np 8 first-mpi.x
// --------------------OR----------------------
// mpirun --use-hwthread-cpus -np 8 first-mpi.x


#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
	int rank,size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	printf("I am %d of %d\n",rank,size);
	MPI_Finalize();
	return 0;
}
