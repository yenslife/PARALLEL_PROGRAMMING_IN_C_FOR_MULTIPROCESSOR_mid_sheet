#include <stdio.h>
#include <string.h> // memset
#include <stdlib.h> // malloc
#include "mpi.h"

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size, n, m;
    int *test = NULL, *part = NULL;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);   //fuck fuck fuck

    char file_name[50];
    // 只有 rank = 0 的 process 會讀取檔案名稱，judge 才不會卡住
    if (rank == 0) {
        scanf("%s", file_name);
    }

    // 將檔案名稱 broadcast 給每個 process
    MPI_Bcast(file_name, 50, MPI_CHAR, 0, MPI_COMM_WORLD); // MPI_Bcast(buffer, count, mpi_datatype, source, comm)

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        if (rank == 0) {
            printf("Could not open file %s\n", file_name);
        }
        MPI_Finalize();
        return 1;
    }

    fscanf(file, "%d %d", &n, &m);

    // 由 n 個部份組成
    part = (int*)malloc(sizeof(int) * n);
    memset(part, 0, sizeof(int) * n); // 初始化

    // 用來記錄每個測試涵蓋了哪些部份
    test = (int*)malloc(sizeof(int) * m * n); // 用分配記憶體的方式來宣告二維陣列，把他當成一維陣列用
    memset(test, 0, sizeof(int) * m * n); // 初始化

    // 讀取資料
    for (int i = 0; i < m; i++) {
        int num, cost;
        fscanf(file, "%d %d", &num, &cost);  // num 為此測試涵蓋的部份的數量，cost 為此測試的成本(成本可以不管，但是 fscanf 一定要讀取到)
        for (int j = 0; j < num; j++) {
            int temp;
            fscanf(file, "%d", &temp); // temp 為此測試涵蓋的部份
            test[i * n + temp - 1] = 1; // temp - 1 即為此測試涵蓋的部份的 index，將此 index 的值設定成 1，表示有涵蓋到
        }
    }

    // 關閉檔案
    fclose(file);

    // 計算每個進程的工作範圍
    int start = rank * (1 << m) / size;  // (rank * 2^m / size) 即為每個進程的工作範圍的起始值
    int end = (rank + 1) * (1 << m) / size;  // ((rank + 1) * 2^m / size) 即為每個進程的工作範圍的結束值

    // printf("size = %d, rank = %d, start = %d, end = %d\n", size, rank, start, end);

    int count = 0;

    // 在這個迴圈中並行處理工作
    for (int i = start; i < end; i++) {     // 這個 i 代表的是測試的組合，例如 m = 3，i = 5，則代表第 5 種測試組合，即 101
        // 程式碼中的其餘部分，與原始版本相同
        memset(part, 0, sizeof(int) * n); // 初始化
        for (int j = 0; j < m; j++) { // 每個測試
            if (i & (1 << j)) {       // 如果這個測試有被選到
                for (int k = 0; k < n; k++) {   // 檢查這個測試涵蓋了哪些部份
                    if (test[j * n + k] == 1)        // 如果有涵蓋到，就把 part 這個陣列的值改成 1
                        part[k] = 1;
                }
            }
        }
        int flag = 1;
        for (int j = 0; j < n; j++) {
            if (part[j] == 0) { // 有部份沒有被涵蓋到
                flag = 0;
                break;
            }
        }
        if (flag == 1)
            count++;
    }

    // 收集每個進程的計數
    int total_count;
    MPI_Reduce(&count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("%d", total_count);
    }

    // 釋放記憶體
    free(test);
    free(part);

    MPI_Finalize();
    return 0;
}
