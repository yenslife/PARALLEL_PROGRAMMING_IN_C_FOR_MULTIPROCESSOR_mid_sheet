#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#define MAX 99999

int main(int argc, char *argv[]) {
	// mpi init
	MPI_Init(&argc, &argv);

	int rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int n = 0;
	int arr[70000];

	if (rank == 0) {
		char filename[50];
		scanf("%s", filename);
		FILE *fp = fopen(filename, "r");
		fscanf(fp, "%d", &n);
		for (int i = 0; i < n; i++) {
			fscanf(fp, "%d", arr + i);
		}
		fclose(fp);
	}
	
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(arr, 70000, MPI_INT, 0, MPI_COMM_WORLD);
	
	int start = rank * n / size;
	int end = (rank + 1) * n / size;

	int min_index = -1;
	int min_value = MAX;
	int min_process = -1;
	int *gather_arr = (int*)malloc(size * sizeof(int));
	int *output_arr = (int*)malloc(n * sizeof(int));

	for (int i = 0; i < n; i++) {
		min_value = MAX;
		for (int j = start; j < end; j++) {
			if (min_value > arr[j]) {
				min_value = arr[j];
				min_index = j;
			}
		}
		// collect
		MPI_Gather(&min_value, 1, MPI_INT, gather_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (rank == 0) {
			for (int j = 0; j < 8; j++) {
				if (min_value >= gather_arr[j]) {
					min_value = gather_arr[j];
					min_process = j;
				}
			}
		}
		MPI_Bcast(&min_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&min_index, 1, MPI_INT, min_process, MPI_COMM_WORLD);
		output_arr[i] = arr[min_index];
		arr[min_index] = MAX;
	}
	if (rank == 0) {
		for (int i = 0; i < n && rank == 0; i++) {
			printf("%d ", output_arr[i]);
		}
	}
	
	MPI_Finalize();
	return 0;
}
