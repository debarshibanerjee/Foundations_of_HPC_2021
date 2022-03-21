#include <math.h>
#include <omp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#if !defined(DOUBLE_PRECISION)
#	define float_t float
#else
#	define float_t double
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

#if defined QUICK_SORT
typedef int(compare_t)(const void*, const void*);
extern inline compare_t compare_ge;
#endif

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
#pragma omp parallel
	{
		int me = omp_get_thread_num();
		unsigned short int seed = (unsigned int)time(NULL) % ((1 << sizeof(short int)) - 1);
		unsigned short int seeds[3] = {seed - me, seed + me, seed + me * 2};

#pragma omp for
		for (int i = 0; i < NDIM; ++i) {
			for (int j = 0; j < n; ++j) {
				x[j].coord[i] = erand48(seeds);
			}
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

// Adapting Luca's QuickSort Parallel Implementation for this case

#if defined QUICK_SORT
int get_median(int start, int end) {
	return (end + start) / 2;
}

inline int compare_ge(const void* A, const void* B) {
	kpoint* a = (kpoint*)A;
	kpoint* b = (kpoint*)B;

	return (a->coord[HOT] >= b->coord[HOT]);
}

int partition(kpoint* x, int start, int end, int axis, compare_t cmp_ge) {
	--end;
	void* pivot = (void*)&x[end].coord[axis];

	int pointbreak = end - 1;
	for (int i = start; i <= pointbreak; i++)
		if (cmp_ge((void*)&x[i].coord[axis], pivot)) {
			while ((pointbreak > i) && cmp_ge((void*)&x[pointbreak].coord[axis], pivot))
				pointbreak--;
			if (pointbreak > i)
				SWAP((void*)&x[i].coord[axis], (void*)&x[pointbreak--].coord[axis], sizeof(kpoint));
		}
	pointbreak += !cmp_ge((void*)&x[pointbreak].coord[axis], pivot);
	SWAP((void*)&x[pointbreak].coord[axis], pivot, sizeof(kpoint));

	return pointbreak;
}

void quicksort(kpoint* x, int start, int end, int axis, compare_t cmp_ge) {
	int size = end - start;

#	if defined DEBUG
	int nthreads = omp_get_num_threads();
	printf("qsort threads %d\n", nthreads);
#	endif

	if (size > 2) {
		int pivot = partition(x, start, end, axis, cmp_ge);
		/* #pragma omp task shared(x) firstprivate(start, pivot) */
		{ quicksort(x, start, pivot, axis, cmp_ge); }

		/* #pragma omp task shared(x) firstprivate(pivot, end) */
		{ quicksort(x, pivot + 1, end, axis, cmp_ge); }
	} else {
		if ((size == 2) && cmp_ge((void*)&x[start].coord[axis], (void*)&x[end - 1].coord[axis])) {
			SWAP((void*)&x[start], (void*)&x[end - 1], sizeof(kpoint));
			/* swap(&x[start], &x[end - 1]); */
		}
	}
}
#endif

kdnode* build_kdtree_serial(kpoint* x, int N, int axis, int depth) {
	int myaxis = choose_splitting_dimension(axis);

	kdnode* node = NULL;
	node = (kdnode*)malloc(sizeof(kdnode));

	node->axis = myaxis;

	if (N == 1 /*|| depth == MAX_DEPTH*/) {
		node->split = *x;
		node->left = NULL;
		node->right = NULL;
		return node;
	}

	if (N == 2) {
		node->split = *x;
		node->left = build_kdtree_serial(x + 1, 1, myaxis, depth);
		node->right = NULL;
		return node;
	}

#if defined DEBUG
	struct timespec ts;
	double tstart = CPU_TIME;
#endif

	kpoint* split_point = quickselect(x, N, N / 2, myaxis);

#if defined DEBUG
	double tend = CPU_TIME;
	printf("Quickselect time: %f\n", tend - tstart);
#endif

	if (split_point) {
		node->split = *split_point;

		kpoint* left_points = x;
		kpoint* right_points = split_point + 1;

		int left_size = split_point - left_points;
		int right_size = x + N - right_points;

		node->left = build_kdtree_serial(left_points, left_size, myaxis, depth);
		node->right = build_kdtree_serial(right_points, right_size, myaxis, depth);
	}
	return node;
}

kdnode* build_kdtree(kpoint* x, int N, int axis, int depth) {
	int myaxis = choose_splitting_dimension(axis);

	kdnode* node = NULL;
	node = (kdnode*)malloc(sizeof(kdnode));

	node->axis = myaxis;

#if defined DEBUG
	printf("%d\n", omp_get_num_threads());
#endif

	if (N == 1) {
		node->split = *x;
		node->left = NULL;
		node->right = NULL;
		return node;
	}

	if (N == 2) {
		node->split = *x;
		node->left = build_kdtree(x + 1, 1, myaxis, depth);
		node->right = NULL;
		return node;
	}

#ifdef QUICK_SORT

#	if defined DEBUG
	struct timespec ts;
	double tstart = CPU_TIME;
#	endif

	int start = 0;
	int end = N - 1;

	int median = get_median(start, end);
	node->split = x[median];
	quicksort(x, start, end, axis, compare_ge);

	kpoint* left_points = x;
	kpoint* right_points = x + median + 1;

	int left_size = median;
	int right_size = N - median - 1;

#	if defined DEBUG
	double tend = CPU_TIME;
	printf("Quicksort time: %f\n", tend - tstart);
#	endif

#endif

#ifndef QUICK_SORT

#	if defined DEBUG
	struct timespec ts;
	double tstart = CPU_TIME;
#	endif

	kpoint* split_point = quickselect(x, N, N / 2, myaxis);

#	if defined DEBUG
	double tend = CPU_TIME;
	printf("Quickselect time: %f\n", tend - tstart);
#	endif

	node->split = *split_point;

	kpoint* left_points = x;
	kpoint* right_points = split_point + 1;

	int left_size = split_point - left_points;
	int right_size = x + N - right_points;

#endif
	/* printf("Threads available: %d\n",omp_get_num_threads()); */
	if (depth >= MAX_DEPTH) {
		node->left = build_kdtree(left_points, left_size, myaxis, depth);
		node->right = build_kdtree(right_points, right_size, myaxis, depth);
		return node;
	} else {
		depth++;
#pragma omp task untied
		node->left = build_kdtree(left_points, left_size, myaxis, depth);
#pragma omp task untied
		node->right = build_kdtree(right_points, right_size, myaxis, depth);
		/* #pragma omp taskwait */
	}
	/* #pragma omp task shared(node) if (depth < MAX_DEPTH) untied */
	/* 		{ node->left = build_kdtree(left_points, left_size, myaxis, depth++); } */
	/* #pragma omp task shared(node) if (depth < MAX_DEPTH) untied*/
	/* 		{ node->right = build_kdtree(right_points, right_size, myaxis, depth++); } */
	/* #pragma omp taskwait */

	return node;
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
	if (argc != 2) {
		printf("Run the program with 1 argument:\n");
		printf("Number of points (dataset size)\n");
		printf("USAGE: ./kdtree.x 1000\n");
		printf("This will create a dataset of 1000 points\n");
		printf("Specify OpenMP threads as follows:\n");
		printf("export OMP_NUM_THREADS=32\n");
		return EXIT_FAILURE;
	}

	int n = atoi(argv[1]);

	/* omp_set_nested(1); */
	/* omp_set_max_active_levels(5);  // 2^5 = 32 == #processors in 1 node */
	omp_set_dynamic(0);

	kpoint* set = (kpoint*)malloc(sizeof(kpoint) * n);
	kpoint_initialize(set, n);

	kdnode* final_nodes;

	int nthreads;
	double tstart = omp_get_wtime();
#pragma omp parallel
	{
		nthreads = omp_get_num_threads();
		/* printf("%d\n", omp_get_num_threads()); */
#pragma omp single nowait
		{
#pragma omp taskgroup
			{ final_nodes = build_kdtree(set, n, 0, 0); }
		}
#pragma omp taskwait
	}
	double tend = omp_get_wtime();

#if defined VERBOSE
	int level = 1, tree_size;
	tree_size = get_size(final_nodes);
	int depth = get_depth(n);
	printf("Size of tree = %d\n", tree_size);
	printf("Is tree size same as required? %s\n", tree_size == n ? "TRUE" : "FALSE");
	printf("Depth of tree = %d\n", depth);
	printf("#threads           time(sec)\n");
#endif
	printf("%d	           %f\n", nthreads, tend - tstart);

	/* printf("debug building complete\n"); */

#if defined PRINT_TREE
	print_tree_file(final_nodes);
#endif

	return 0;
}
