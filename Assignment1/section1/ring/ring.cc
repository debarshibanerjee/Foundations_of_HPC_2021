#include <cstdlib>
#include <fstream>
#include <iostream>

#define OMPI_SKIP_MPICXX 1
#include <mpi.h>
#define USE MPI

#define PRINT_RESULT                                                                               \
	"I am processor " << rank << " and I have received " << total_msg_recvd << " messages.\n"      \
					  << "My final messages have tags " << itag_recv_left                          \
					  << " from the left, and " << itag_recv_right << " from the right.\n"         \
					  << "At least one of them should have the tag " << itag_og                    \
					  << " to fulfil the condition of receiving back the original message.\n"      \
					  << "They have values +" << imsgleft << " from the left, and " << imsgright   \
					  << " from the right.\n"                                                      \
					  << "The total time taken (not including the time taken for writing to "      \
						 "file and printing on-screen) is "                                        \
					  << t_end - t_start << " seconds.\n"
#define ITER 1'000'000
#define EXIT_CONDITION itag_recv_left != itag_og or itag_recv_right != itag_og
/* #define EXIT_CONDITION itag_recv_left != itag_og and itag_recv_right != itag_og */
/* #define EXIT_CONDITION abs(imsgleft) != abs(imsgright) */
/* #define EXIT_CONDITION counter<10000 */

using std::cout;
using std::endl;

int main(int argc, char** argv) {
	int size;
	int rank;
	double t_start, t_end, t_sum{0}, t_avg;
	MPI_Status status_l;
	MPI_Status status_r;
	// MPI_Request request;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int itag_og{10 * rank}, itag_send, itag_send_left, itag_send_right;
	int itag_recv_left, itag_recv_right;
	int imsgleft;
	int imsgright;
	int omsgleft;
	int omsgright;
	int total_msg_recvd;
	int counter;

	// this gives us '0' as right of '3', '3' as left of '0'
	// without adding '+size' it doesn't work due to the edge case of -ve numbers.
	int left{(rank - 1 + size) % size};
	int right{(rank + 1 + size) % size};
	/* cout << left << " " << rank << " " << right << endl; */
	// the same can be accomplished as shown below
	// it is much more explicit and tedious as we specify the edge-cases manually
	/* int left{rank - 1}; */
	/* int right{rank + 1}; */
	/* if (rank == 0) { */
	/* 	left = size - 1; */
	/* } */
	/* if (rank == (size - 1)) { */
	/* 	right = 0; */
	/* } */

	for (int k = 0; k < ITER; ++k) {
		imsgleft = 0;
		imsgright = 0;
		omsgleft = -rank;
		omsgright = +rank;
		total_msg_recvd = 0;
		counter = 0;
		t_start = MPI_Wtime();
		do {
			if (counter == 0) {
				itag_send = itag_og;
				itag_send_left = itag_send;
				itag_send_right = itag_send;
			}

			MPI_Send(&omsgleft, 1, MPI_INT, left, itag_send_left, MPI_COMM_WORLD);
			MPI_Recv(&imsgright, 1, MPI_INT, right, MPI_ANY_TAG, MPI_COMM_WORLD, &status_r);
			MPI_Send(&omsgright, 1, MPI_INT, right, itag_send_right, MPI_COMM_WORLD);
			MPI_Recv(&imsgleft, 1, MPI_INT, left, MPI_ANY_TAG, MPI_COMM_WORLD, &status_l);

			/* cout << "rank = " << rank << " and RIGHT tag = " << status_r.MPI_TAG << endl; */
			/* cout << "rank = " << rank << " and LEFT tag = " << status_l.MPI_TAG << endl; */

			itag_recv_left = status_l.MPI_TAG;
			itag_recv_right = status_r.MPI_TAG;
			itag_send_left = itag_recv_right;
			itag_send_right = itag_recv_left;

			omsgleft = imsgright - rank;
			omsgright = imsgleft + rank;

			total_msg_recvd += 2;
			++counter;
			/* cout << counter << endl; */
		} while (EXIT_CONDITION);
		t_end = MPI_Wtime();  // end time counter BEFORE printing on-screen or writing to file
		t_sum = t_sum + t_end - t_start;
	}
	t_avg = t_sum / ITER;
	// print on screen
	/* cout << PRINT_RESULT << endl; */

	// write to file
	std::ofstream file;
	file.open("ring.dat", std::ios::app);
	file << PRINT_RESULT << endl;
	file.close();

	if (rank == 0) {
		cout << t_avg << endl;
	}

	MPI_Finalize();
}
