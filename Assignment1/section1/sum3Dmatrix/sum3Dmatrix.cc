#include <climits>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

#define OMPI_SKIP_MPICXX 1
#include <mpi.h>
#define USE MPI

#define ITER 50

// Question:
//     implement a simple 3d matrix-matrix addition in parallel using a 1D,2D and 3D distribution
//     of data using virtual topology and study its scalability in term of communication within a
//     single THIN node. Use here just collective operations to communicate among MPI processes.
//     Program should accept as input the sizes of the matrixes and should then allocate the matrix
//     and initialize them using double precision random numbers.  Model the network performance and
//     try to identify which the best distribution given the topology of the node you are using for
//     the following sizes:  - 2400 x 100 x 100 ; 1200 x 200 x 100 ;  800 x 300 x 100;
//     Discuss performance for the three domains in term of 1D/2D/3D distribution keeping the number
//     of processor constant at 24. Provide a table with all possible distribution on 1 D 2D and 3D
//     partition and report the timing.

// HOW TO USE?
// mpirun -np "..." ./executable size1 size2 size3 number_of_dimensions dim1 dim2 dim3
// On laptop (for testing):
// mpic++ sum3Dmatrix.cc -o sum3Dmatrix.x -Wall -Wextra
// mpirun -np 4 ./sum3Dmatrix.x 4 4 4 3 4 1 1 2>matrix.log 1>matrix.dat
// On server:
// mpic++ sum3Dmatrix.cc -o sum3Dmatrix.x -Wall -Wextra
// mpirun -np 24 ./sum3Dmatrix.x 2400 100 100 3 4 3 2 2>matrix.log 1>matrix.dat

using std::cout;
using std::endl;

int main(int argc, char** argv) {
	int period{1};
	int reorder{1};
	int nproc;
	int rank;
	int err{33};
	double t_start, t_end, t_sum{0}, t_avg;
	/* double t_init_start, t_init_end; */

	const size_t size1{std::stoull(argv[1])};
	const size_t size2{std::stoull(argv[2])};
	const size_t size3{std::stoull(argv[3])};
	size_t n = size1 * size2 * size3;

	/* MPI_Status status; */
	/* MPI_Request request; */

	MPI_Init(&argc, &argv);
	MPI_Comm cart_comm;
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (n % nproc != 0) {
		n = nproc * ceil((double)n / nproc);
		/* if (rank == 0) { */
		/* cout << "Exiting. The total size of matrix should be multiple of number of "*/
		/* 		"processors used to run the program." */
		/* 	 << endl; */
		/* } */
		/* MPI_Abort(MPI_COMM_WORLD, err); */
		/* MPI_Finalize(); */
		/* return err; */
		/* } */
	}

	const int ndims{std::stoi(argv[4])};
	int dims[ndims];

	dims[0] = std::stoi(argv[5]);

	if (ndims == 2) {
		dims[1] = std::stoi(argv[6]);
	}

	if (ndims == 3) {
		dims[1] = std::stoi(argv[6]);
		dims[2] = std::stoi(argv[7]);
	}

	MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, &period, reorder, &cart_comm);
	MPI_Comm_rank(cart_comm, &rank);

	double* matrix_1 = new double[n];
	double* matrix_2 = new double[n];
	double* matrix_3 = new double[n];

	size_t size_div{n / nproc};

	double matrix_1_div[size_div];
	double matrix_2_div[size_div];
	double matrix_3_div[size_div];

	for (int l = 0; l < ITER; ++l) {
		t_start = MPI_Wtime();
		if (rank == 0) {
			/* t_init_start = MPI_Wtime(); */
			std::srand(static_cast<unsigned>(time(nullptr)));
			/* std::random_device rd; */
			/* std::mt19937_64 gen(rd()); */
			/* std::uniform_real_distribution<> dis(INT_MIN, INT_MAX); */
			// change the values to arbitrarily low or high numbers as needed
			// to generate random numbers in that range
			for (size_t i = 0; i < size1; ++i) {
				for (size_t j = 0; j < size2; ++j) {
					for (size_t k = 0; k < size3; ++k) {
						/* matrix_1[k + (j * size3) + (i * size2 * size3)] = dis(gen); */
						/* matrix_2[k + (j * size3) + (i * size2 * size3)] = dis(gen); */
						matrix_1[k + (j * size3) + (i * size2 * size3)] = rand();
						matrix_2[k + (j * size3) + (i * size2 * size3)] = rand();
					}
				}
			}
			/* t_init_end = MPI_Wtime(); */
			/* cout << "Time to initialize the matrix on 1 thread = " << t_init_end -
			 * t_init_start
			 * << endl; */
		}

		MPI_Scatter(matrix_1, size_div, MPI_DOUBLE, matrix_1_div, size_div, MPI_DOUBLE, 0,
					cart_comm);
		MPI_Scatter(matrix_2, size_div, MPI_DOUBLE, matrix_2_div, size_div, MPI_DOUBLE, 0,
					cart_comm);

		for (size_t i = 0; i < size_div; ++i) {
			matrix_3_div[i] = matrix_1_div[i] + matrix_2_div[i];
		}

		MPI_Gather(matrix_3_div, size_div, MPI_DOUBLE, matrix_3, size_div, MPI_DOUBLE, 0,
				   cart_comm);

		t_end = MPI_Wtime();
		t_sum = t_sum + (t_end - t_start);
	}
	t_avg = t_sum / ITER;

	// print stuff to verify
	// commented out for final version
	if (rank == 0) {
		/* cout << "Time to complete the program = " << t_end - t_start << endl; */
		cout << t_avg << endl;
		/* cout << "array 1" << endl; */
		/* size_t counter{0}; */
		/* for (size_t i = 0; i < size1; ++i) { */
		/* 	for (size_t j = 0; j < size2; ++j) { */
		/* 		for (size_t k = 0; k < size3; ++k) { */
		/* 			cout << std::setprecision(std::numeric_limits<double>::max_digits10 + 1) */
		/* 				 << matrix_1[k + (j * size3) + (i * size2 * size3)] << " "; */
		/* 			++counter; */
		/* 		} */
		/* 	} */
		/* } */
		/* cout << endl; */
		/* cout << "Total Elements " << counter << endl; */
		/* cout << endl; */
		/* cout << "array 2" << endl; */
		/* counter = 0; */
		/* for (size_t i = 0; i < size1; ++i) { */
		/* 	for (size_t j = 0; j < size2; ++j) { */
		/* 		for (size_t k = 0; k < size3; ++k) { */
		/* 			cout << std::setprecision(std::numeric_limits<double>::max_digits10 + 1) */
		/* 				 << matrix_2[k + (j * size3) + (i * size2 * size3)] << " "; */
		/* 			++counter; */
		/* 		} */
		/* 	} */
		/* } */
		/* cout << endl; */
		/* cout << "Total Elements " << counter << endl; */
		/* cout << endl; */
		/* cout << "sum" << endl; */
		/* counter = 0; */
		/* for (size_t i = 0; i < size1; ++i) { */
		/* 	for (size_t j = 0; j < size2; ++j) { */
		/* 		for (size_t k = 0; k < size3; ++k) { */
		/* 			cout << std::setprecision(std::numeric_limits<double>::max_digits10 + 1) */
		/* 				 << matrix_3[k + (j * size3) + (i * size2 * size3)] << " "; */
		/* 			++counter; */
		/* 		} */
		/* 	} */
		/* } */
		/* cout << endl; */
		/* cout << "Total Elements " << counter << endl; */
		/* cout << endl; */
	}

	MPI_Finalize();

	return 0;
}
