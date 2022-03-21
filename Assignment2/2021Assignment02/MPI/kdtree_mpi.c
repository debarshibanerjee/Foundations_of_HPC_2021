#include "mpi.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#if !defined(DOUBLE_PRECISION)
	#define float_t float
	#define mpi_float_t MPI_FLOAT
#else
	#define float_t double
	#define mpi_float_t MPI_DOUBLE
#endif

#define NDIM 2
#define K NDIM
#define HOT 0
#define ROOT 0
#define MAX_DEPTH 5

#define CPU_TIME (clock_gettime(CLOCK_REALTIME, &ts), (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

#define CPU_TIME_th                                                                                \
	(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &myts),                                                \
	 (double)myts.tv_sec + (double)myts.tv_nsec * 1e-9)

#define SWAP(A, B, SIZE)                                                                           \
	do {                                                                                           \
		int sz = (SIZE);                                                                           \
		char* a = (A);                                                                             \
		char* b = (B);                                                                             \
		do {                                                                                       \
			char _temp = *a;                                                                       \
			*a++ = *b;                                                                             \
			*b++ = _temp;                                                                          \
		} while (--sz);                                                                            \
	} while (0)

typedef struct kdnode kdnode;
typedef struct kpoint kpoint;

struct kpoint {
	float_t coord[NDIM];
};

struct kdnode {
	int axis;
	kpoint split;
	kdnode *left, *right;
};

void swap(kpoint* x, kpoint* y) {
	kpoint tmp = *x;
	tmp = *x;
	*x = *y;
	*y = tmp;
}

void kpoint_initialize(kpoint* x, int n) {
	srand48((unsigned int)time(NULL));
	for (int i = 0; i < NDIM; ++i) {
		for (int j = 0; j < n; ++j) {
			x[j].coord[i] = drand48();
		}
	}
}

int choose_splitting_dimension(int axis) {
	return (axis + 1) % NDIM;
}

int get_size(kdnode* node) {
	if (node == NULL)
		return 0;
	else
		return (get_size(node->left) + 1 + get_size(node->right));
}

int get_depth(int N) {
	if (N == 0) {
		return 0;
	}
	return floor(log2(N)) + 1;
}

int get_dest_proc(int rank, int nproc, int depth) {
	return rank + (nproc / pow(2, depth + 1));
}

kpoint* quickselect(kpoint* x, int N, int k, int axis) {
	int i, st;
	for (st = i = 0; i < N - 1; i++) {
		if (x[i].coord[axis] > x[N - 1].coord[axis])
			continue;
		swap(x + i, x + st);
		st++;
	}
	swap(x + N - 1, x + st);
	return k == st
			   ? x + st
			   : (st > k ? quickselect(x, st, k, axis) : quickselect(x + st, N - st, k - st, axis));
}

kdnode* build_kdtree(kpoint* x, int N, int axis, int depth) {
	if (N == 0)
		return NULL;

	int rank, nproc;
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Datatype MPI_kpoint;
	MPI_Type_contiguous(sizeof(kpoint), MPI_BYTE, &MPI_kpoint);
	MPI_Type_commit(&MPI_kpoint);

	int myaxis = choose_splitting_dimension(axis);

	kdnode* node = NULL;
	node = (kdnode*)malloc(sizeof(kdnode));

	node->axis = myaxis;

	if (N == 1) {
		node->split = *x;
		node->left = NULL;
		node->right = NULL;
		return node;
	}

	if (N == 2) {
		node->split = *x;
		node->left = build_kdtree(x + 1, 1, myaxis, depth + 1);
		node->right = NULL;
		return node;
	}

	kpoint* split_point = quickselect(x, N, N / 2, myaxis);

	node->split = *split_point;

	kpoint* left_points = x;
	kpoint* right_points = split_point + 1;

	int left_size = split_point - left_points;
	int right_size = x + N - right_points;

	if (depth < log2(nproc)) {
		int tmp_depth = depth + 1;
		int destination_proc = get_dest_proc(rank, nproc, depth);
		/* printf("source: %d, dest : %d, depth : %d\n", rank, destination_proc, depth); */
		MPI_Send(&myaxis, 1, MPI_INT, destination_proc, 13 * rank, MPI_COMM_WORLD);
		MPI_Send(&tmp_depth, 1, MPI_INT, destination_proc, 15 * rank, MPI_COMM_WORLD);
		MPI_Send(&right_size, 1, MPI_INT, destination_proc, 17 * rank, MPI_COMM_WORLD);
		MPI_Send(right_points, right_size, MPI_kpoint, destination_proc, 19 * rank, MPI_COMM_WORLD);
		node->left = build_kdtree(left_points, left_size, myaxis, depth + 1);
		kdnode* empty_node = (kdnode*)malloc(sizeof(kdnode));
		empty_node->left = NULL;
		empty_node->right = NULL;
		empty_node->axis = -1;
		node->right = empty_node;
	} else {
		node->left = build_kdtree(left_points, left_size, myaxis, log2(nproc));
		node->right = build_kdtree(right_points, right_size, myaxis, log2(nproc));
	}

	return node;
}

kdnode* init_build_kdtree() {
	MPI_Status* status;
	int local_axis;
	int local_depth;
	int local_right_size;
	// the right sub tree is sent to a different processor to start building

	MPI_Datatype MPI_kpoint;
	MPI_Type_contiguous(sizeof(kpoint), MPI_BYTE, &MPI_kpoint);
	MPI_Type_commit(&MPI_kpoint);

	MPI_Recv(&local_axis, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	MPI_Recv(&local_depth, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	MPI_Recv(&local_right_size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);

	kpoint* local_points = (kpoint*)malloc(local_right_size * sizeof(kpoint));

	MPI_Recv(local_points, local_right_size, MPI_kpoint, MPI_ANY_SOURCE, MPI_ANY_TAG,
			 MPI_COMM_WORLD, status);

	kdnode* partial_node = build_kdtree(local_points, local_right_size, local_axis, local_depth);

	return partial_node;
}

void export_tree(kdnode* x, FILE* fp) {
	if ((x->right == NULL) && (x->left == NULL)) {
		return;
	}

	if (x->right == NULL) {
		kpoint a = x->split;
		kdnode* left = x->left;
		kpoint left_point = left->split;
		fprintf(fp, " \"%f,%f\" -> \"%f,%f\" [label=%c] \n", a.coord[0], a.coord[1],
				left_point.coord[0], left_point.coord[1], x->axis + 88);
		// 88 to get the ASCII number for X (88)and Y (89)
		export_tree(left, fp);
		return;
	}

	if (x->left == NULL) {
		kpoint a = x->split;
		kdnode* right = x->right;
		kpoint right_point = right->split;
		fprintf(fp, " \"%f,%f\" -> \"%f,%f\" [label=%c]\n", a.coord[0], a.coord[1],
				right_point.coord[0], right_point.coord[1], x->axis + 88);
		export_tree(right, fp);
		return;
	}

	kpoint a = x->split;
	kdnode* right = x->right;
	kdnode* left = x->left;
	kpoint right_point = right->split;
	kpoint left_point = left->split;

	fprintf(fp, " \"%f,%f\" -> \"%f,%f\" [label=%c] \n", a.coord[0], a.coord[1],
			right_point.coord[0], right_point.coord[1], x->axis + 88);
	fprintf(fp, " \"%f,%f\" -> \"%f,%f\" [label=%c] \n", a.coord[0], a.coord[1],
			left_point.coord[0], left_point.coord[1], x->axis + 88);

	export_tree(left, fp);
	export_tree(right, fp);
	return;
}

void print_tree_file(kdnode* x) {
	FILE* fp;
	fp = fopen("tree.gv", "w");
	fprintf(fp, "digraph kdtree {\n");
	export_tree(x, fp);
	fprintf(fp, "}");
	fclose(fp);
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	int nproc, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Datatype MPI_kpoint;
	MPI_Type_contiguous(sizeof(kpoint), MPI_BYTE, &MPI_kpoint);
	MPI_Type_commit(&MPI_kpoint);

	int n = atoi(argv[1]);

	if (argc != 2) {
		if (rank == ROOT) {
			printf("Run the program with 1 argument:\n");
			printf("Number of points (dataset size)\n");
			printf("USAGE: mpirun -np 4 ./kdtree_mpi.x 1000\n");
			printf("This will create a dataset of 1000 points using 4 MPI processes\n");
		}
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		MPI_Finalize();
		return EXIT_FAILURE;
	}

	kpoint* set;
	kdnode* final_nodes;
	double t_start, t_end, t_total, t_max, t_min, t_avg;
	t_start = MPI_Wtime();

	if (rank == ROOT) {
		set = (kpoint*)malloc(sizeof(kpoint) * n);
		kpoint_initialize(set, n);
		final_nodes = build_kdtree(set, n, -1, 0);
	} else {
		final_nodes = init_build_kdtree();
	}

	t_end = MPI_Wtime();
	t_total = t_end - t_start;
	MPI_Reduce(&t_total, &t_max, 1, MPI_DOUBLE, MPI_MAX, ROOT, MPI_COMM_WORLD);
	MPI_Reduce(&t_total, &t_min, 1, MPI_DOUBLE, MPI_MIN, ROOT, MPI_COMM_WORLD);
	MPI_Reduce(&t_total, &t_avg, 1, MPI_DOUBLE, MPI_SUM, ROOT, MPI_COMM_WORLD);

#if defined VERBOSE
	int level = 1, tree_size;
	tree_size = get_size(final_nodes);
	int depth = get_depth(n);
	if (rank == ROOT) {
		t_avg = t_avg / nproc;
		printf("Size of tree = %d\n", tree_size);
		printf("Is tree size same as required? %s\n", tree_size == n ? "TRUE" : "FALSE");
		printf("NOTE: In case of MPI run it may not be the same as some extra NULL nodes are added along the way\n");
		printf("Depth of tree = %d\n", depth);
		printf(
			"#threads           avg_time(sec)       min_time(sec)           "
			"max_time(sec)\n");
	}
#endif
	if (rank == ROOT) {
		t_avg = t_avg / nproc;
		printf("%d	           %f	          %f 	         %f\n", nproc, t_avg, t_min, t_max);
	}

	/* printf("debug building complete\n"); */

#if defined PRINT_TREE
	if (rank == ROOT) {
		print_tree_file(final_nodes);
	}
#endif

	MPI_Finalize();
	return 0;
}
