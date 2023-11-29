#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int t, n, m, D;
	int *K, *At;
	char filename[50];
    if (rank == 0) {
		scanf("%s", filename);
		FILE *f = fopen(filename, "r");
        // t, n, m
        fscanf(f, "%d", &t);
        fscanf(f, "%d %d", &n, &m);

        // A_t (A0)
        At = (int *)malloc(n * m * sizeof(int));
        for (int i = 0; i < n * m; i++) {
            fscanf(f , "%d", &At[i]);
        }

        // K
        fscanf(f, "%d", &D);
		K = (int*)malloc(sizeof(int) * D * D);
        for (int i = 0; i < D; i++) {
            for (int j = 0; j < D; j++) {
                fscanf(f, "%d", &K[i * D + j]);
            }
        }
    }
	// bcast
	MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&D, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// malloc
	if (rank != 0) {
		At = (int *)malloc(n * m * sizeof(int));
		K = (int *)malloc(D * D * sizeof(int));
	}
	// bcast (array)
	MPI_Bcast(At, n * m, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(K, D * D, MPI_INT, 0, MPI_COMM_WORLD);
//////////////////////////讀檔案資料初始化完成/////////////////////////////////

	// 分配 rank 計算 buffer size
	int start = rank * n / size;
	int end = (rank + 1) * n / size;
	int send_buf_size = (end - start) * m;

	// 準備要給 MPI_Gatherv 用的參數
	int *displs, *recvCount;
	displs = (int *)malloc(size * sizeof(int));    // 每個 process 傳送資料大小的的偏移量 (array)
	recvCount = (int *)malloc(size * sizeof(int)); // 每個 process 的資料大小             (array)
	displs[0] = 0;
	for (int i = 0; i < size; i++) {
		int tmp = 0;
		tmp = ((i + 1) * n / size - i * n / size) * m;
		if (i + 1 != size) displs[i + 1] = displs[i] + tmp; // 避免存取到外面
		recvCount[i] = tmp;
	}

	int *At_next = (int *)malloc(send_buf_size * sizeof(int));

	// start computing
	for (int time = 0; time < t; time++) {
		int indexI = 0;
		for (int i = start; i < end; i++) {
			for (int j = 0; j < m; j++) {
				int sum = 0;
				for (int di = -(D - 1) / 2; di <= (D - 1) / 2; di++) {
					for (int dj = -(D - 1) / 2; dj <= (D - 1) / 2; dj++) {
						int ni = (i + di + n) % n;
						int nj = (j + dj + m) % m;
						int ki = (di + (D - 1) / 2);
						int kj = (dj + (D - 1) / 2);
						sum += K[ki * D + kj] * At[ni * m + nj];
					}
				}
				At_next[indexI * m + j] = sum / (D * D) + 0.5; // update current At
			}
			indexI++;
		}

		// 將結果送給 rank 0 的 buffer，並在 rank 0 將 buffer 的東西移動到 At
		MPI_Gatherv(At_next, send_buf_size, MPI_INT, At, recvCount, displs, MPI_INT, 0, MPI_COMM_WORLD);
		// 廣播 At 給所有 process
		MPI_Bcast(At, n * m, MPI_INT, 0, MPI_COMM_WORLD);

	}

	// 结果
	if (rank == 0) {
		for (int i = 0; i < n * m; i++) {
			printf("%d ", At[i]);
		}
	}

	// final
	free(At);
	free(At_next);
	free(K);
    MPI_Finalize();

    return 0;
}
