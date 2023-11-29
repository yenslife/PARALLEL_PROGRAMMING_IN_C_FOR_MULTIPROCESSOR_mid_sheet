#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<string.h>
#include<stdbool.h>
#define MAX 50000
#include <unistd.h> // sleep test

// 回傳目前最短的距離
int min_dist(int dist[], int ok[], int n) {
	int min = MAX, min_index = -1; // 重點是這個，有可能會有選不到點的問題
	for (int v = 0; v < n; v++) {
		if (ok[v] == 0 && dist[v] < min) {
			min = dist[v];
			min_index = v;
			// u 是 n 的情況
		}
	}
	return min_index;
}

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);

	long n;
	int rank, size;
	char filename[50];
	short *graph = NULL;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	if (rank == 0) {
		scanf("%s", filename);
		FILE* fp = fopen(filename, "r");
		fscanf(fp, "%ld", &n);
		long i, j;  // 下面太大的相乘可能會 overflow 變成負的所以要用 unsigned
		short value;
		graph = (short *)malloc(sizeof(short) * n * n);
		while(fscanf(fp, "%ld %ld %hd", &i, &j, &value) != EOF) {
			graph[n * i + j] = value;
		}
		fclose(fp);
	}
	MPI_Bcast(&n, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	if (rank != 0) {
		graph = (short *)malloc(sizeof(short) * n * n / size);
		for (int i = 0; i < n * n / size; i++) graph[i] = 0;
	}

	// 會不會從一開始就不用讓所有的 process 都有一個 dist array 呢？
	// 沒有平行化的 code

	//int ok[n];
	//int dist[n];
	int *ok = (int *)malloc(sizeof(int) * n);
	int *dist = (int *)malloc(sizeof(int) * n);

	for (int i = 0; i < n; i++) {
		ok[i] = 0, dist[i] = MAX;
	}

	if (rank == 0 && n != MAX) {

		dist[0] = 0;

		for (int count = 0; count < n; count++) { // 處理剩下 n-1 個點
			int u = min_dist(dist, ok, n);
			ok[u] = 1;
			for (int v = 0; v < n; v++) {
				if (!ok[v] && graph[u * n + v] && dist[u] + graph[u * n + v] < dist[v]) // 他是有向圖，所以只能這樣寫
					dist[v] = dist[u] + graph[u * n + v];
			}
		}

		for (int i = 0; i < n; i++) {
			printf("%d ", dist[i]);
		}
	}

	if (n == MAX && 0) {
		// 平分給大家，找到 graph[u * n + v] < dist[v]，可能要用直的切
		if (rank == 0) {
			for (int rank_i = 1; rank_i < size; rank_i++) {
				for (int j = 0; j < n; j++) {
					MPI_Send(graph + rank_i * (n / size) + j * n, n / size, MPI_SHORT, rank_i, 0, MPI_COMM_WORLD);
				}
			}
		} else {
			for (int j = 0; j < n; j++) {
				MPI_Recv(graph + j * (n / size), n / size, MPI_SHORT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}

		short *new_graph = (short *)malloc(sizeof(short) * n * n / size); // for rank 0
		if (rank == 0) {
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n / size; j++) {
					new_graph[i * (n / size) + j] = graph[i * n + j];
				}
			}
			free(graph);
			graph = (short *)malloc(sizeof(short) * n * n / size);
			for (int i = 0; i < n * n / size; i++) graph[i] = new_graph[i];
		}
		free(new_graph);

		dist[0] = 0;

		for (int count = 0; count < n - 1; count++) {
			int u = min_dist(dist, ok, n);
			if (u == -1) printf("error");
			ok[u] = 1;
			int v_start = rank * n / size;
			int v_end = (rank + 1) * n / size;
			for (int v = v_start; v < v_end; v++) {  // 我發現 rank 0 的排序方式會和其他不一樣
				if (!ok[v] && graph[u * n/size + (v - v_start)] && dist[u] + graph[u * n/size + (v - v_start)] < dist[v]) {
					dist[v] = dist[u] + graph[u * n/size + (v - v_start)];
				}
			}
			// 把每一個 process 有更改到的相鄰的點 dist 找最小ㄉ，最後再同步更新大家的 dist
			int *tmp_dist = (int *)malloc(sizeof(int) * n);
			for (int i = 1; i < size; i++) {
				if (rank == i) {
					MPI_Send(dist, n, MPI_INT, 0, 0, MPI_COMM_WORLD);
				} else if (rank == 0) {
					for (int j = 0; j < n; j++) tmp_dist[j] = MAX;
					MPI_Recv(tmp_dist, n, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					for (int j = 0; j < n; j++) {
						if (tmp_dist[j] < dist[j]) {
							dist[j] = tmp_dist[j];
						}
					}
				}
			}
			free(tmp_dist);
			MPI_Bcast(dist, n, MPI_INT, 0, MPI_COMM_WORLD);
		}
		if (rank == 0) {
			for (int i = 0; i < n; i++) printf("%d ", dist[i]);
		}
	}

	// final
	free(graph);
	free(ok);
	free(dist);
	MPI_Finalize();

	return 0;
}
