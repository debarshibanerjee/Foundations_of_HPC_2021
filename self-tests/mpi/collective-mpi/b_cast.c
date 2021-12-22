#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int num_elements = 2;
	MPI_Init(NULL, NULL);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int* data = (int*)calloc(num_elements, sizeof(int));
	assert(data != NULL);

	printf("Before Broadcast\n");
	printf("Rank - %d, data - %d, %d\n", world_rank, data[0], data[1]);

	MPI_Barrier(MPI_COMM_WORLD);

	if (world_rank == 0) {
		data[0] = 2;
		data[1] = 4;
	}
	MPI_Bcast(data, num_elements, MPI_INT, 0, MPI_COMM_WORLD);

	printf("After Broadcast\n");

	printf("Rank - %d, data - %d, %d\n", world_rank, data[0], data[1]);

	free(data);
	MPI_Finalize();
}
