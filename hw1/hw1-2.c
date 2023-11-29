#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#define str_len 50

/* 這是一個凸包問題 可以用外積來判斷順時針還是逆時針 題目要求順時針 */

// 用來找出最左邊的點
bool compare(int x1, int y1, int x2, int y2) {
    return (x1 < x2) || (x1 == x2 && y1 < y2);
}

// 判斷是否順時針使用向量外積行列式 若 > 0 則 01 -> 02 是順時針
double cross(int x0, int y0, int x1, int y1, int x2, int y2) {
    return (double)(x1 - x0) * (double)(y2 - y0) - (double)(x2 - x0) * (double)(y1 - y0);          // v1 x v2
}

// 計算兩點距離的平方
double len(int x1, int y1, int x2, int y2) {
    return (double)(x1 - x2) * (double)(x1 - x2) + (double)(y1 - y2) * (double)(y1 - y2);       // |v1|^2
}

// 比較兩點距離的平方，以 0 為基準，若 01 > 02 則回傳 true
bool compare_len(int x0, int y0, int x1, int y1, int x2, int y2) {
    return len(x0, y0, x1, y1) > len(x0, y0, x2, y2);
}

int main (int argc, char *argv[]) {

    // mpi init
    MPI_Init(&argc, &argv);

    int rank, size, num_of_lines, start_point = 0;
    int *coords = NULL;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // if rank is 0, read file name, num of lines, and coords
    if (rank == 0) {
        char file_name[str_len];
        scanf("%s", file_name);
        FILE *fp;
        fp = fopen(file_name, "r");
        fscanf(fp, "%d", &num_of_lines);
        coords = (int *)malloc(sizeof(int) * num_of_lines * 2);
        for (int i = 0; i < num_of_lines; i++)
            fscanf(fp, "%d %d", coords + i * 2, coords + i * 2 + 1);  // x, y
        fclose(fp);
    }

    // mpi broadcast
    MPI_Bcast(&num_of_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);         // 把 num_of_lines 的資料傳給每個 rank

    if (rank != 0)
        coords = (int *)malloc(sizeof(int) * num_of_lines * 2);

    MPI_Bcast(coords, num_of_lines * 2, MPI_INT, 0, MPI_COMM_WORLD); // 把 coords 的資料傳給每個 rank

    // 找到起始點(最左)
	// 分配 mpi 起點
	int start = rank * num_of_lines / size;
	int end = (rank + 1) * num_of_lines / size;
	int send_data[1];
	int *tmp_arr = (int*)malloc(size * sizeof(int));
	for (int i = start; i < end; i++) {
		int x1 = coords[i * 2], y1 = coords[i * 2 + 1];
		int x2 = coords[start_point * 2], y2 = coords[start_point * 2 + 1];
        if (compare(x1, y1, x2, y2)) {
			start_point = i;
		}
	}
	send_data[0] = start_point;
	MPI_Gather(send_data, 1, MPI_INT, tmp_arr, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (rank == 0) {
		int index;
		for (int i = 0; i < size; i++) {
			index = tmp_arr[i];
			int x1 = coords[index * 2], y1 = coords[index * 2 + 1];
			int x2 = coords[start_point * 2], y2 = coords[start_point * 2 + 1];
			if (compare(x1, y1, x2, y2)) start_point = tmp_arr[i];
		}
	}
	free(tmp_arr);
    MPI_Bcast(&start_point, 1, MPI_INT, 0, MPI_COMM_WORLD);          // 把 start_point 的資料傳給每個 rank

    /////////////////////////////// 初始化結束 ///////////////////////////////////


    if (rank == 0) {
        int current_point = start_point;
        while(true) {
            printf("%d ", current_point+1);
            int next_point = current_point;
            for (int j = 0; j < num_of_lines; j++) {
                double c = cross(coords[current_point * 2], coords[current_point * 2 + 1], coords[j * 2], coords[j * 2 + 1], coords[next_point * 2], coords[next_point * 2 + 1]);
                if (c < 0 || (c == 0 && compare_len(coords[current_point * 2], coords[current_point * 2 + 1], coords[j * 2], coords[j * 2 + 1], coords[next_point * 2], coords[next_point * 2 + 1]))) {
                    next_point = j;
                }
            }
            if (next_point == start_point) break;
            current_point = next_point;
        }
    }





    /////////////////////////////////最後階段/////////////////////////////////////
    // free memory
    free(coords);

    // mpi finalize
    MPI_Finalize();

    return 0;
}
