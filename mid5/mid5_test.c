// use gcc test.c -lm  # to link math library

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_POINTS 100

typedef struct {
    int x, y;
} Point;

typedef struct {
    int node;
    double cost;
} Edge;

double euclidean_distance(Point p1, Point p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

int compare_edges(const void *a, const void *b) {
    return ((Edge *)a)->cost > ((Edge *)b)->cost;
}

double prim(Point points[MAX_POINTS], int n) {
    int visited[MAX_POINTS] = {0};
    Edge edges[MAX_POINTS];
    int edge_count = 0;
    double total_cost = 0.0;

    visited[0] = 1; // Start with the first point

    // Initialize edges with distances from the first point
    for (int i = 1; i < n; i++) {
        edges[edge_count].node = i;
        edges[edge_count].cost = euclidean_distance(points[0], points[i]);
        edge_count++;
    }

    qsort(edges, edge_count, sizeof(Edge), compare_edges);

    while (edge_count > 0) {
        int current = edges[0].node;
        double cost = edges[0].cost;

        // Remove the smallest edge from the heap
        for (int i = 0; i < edge_count - 1; i++) {
            edges[i] = edges[i + 1];
        }
        edge_count--;

        if (!visited[current]) {
            visited[current] = 1;
            total_cost += cost;

            // Update the heap with new edges from the current node
            for (int i = 0; i < n; i++) {
                if (!visited[i]) {
                    edges[edge_count].node = i;
                    edges[edge_count].cost = euclidean_distance(points[current], points[i]);
                    edge_count++;
                }
            }

            qsort(edges, edge_count, sizeof(Edge), compare_edges);
        }
    }

    return total_cost;
}

int main() {
    int n;
    scanf("%d", &n);

    Point points[MAX_POINTS];
    for (int i = 0; i < n; i++) {
        scanf("%d %d", &points[i].x, &points[i].y);
    }

    double cost = prim(points, n);
	cost = (int)(cost * 10000) * 0.0001; // 相當於無條件捨去

    // Output the cost with four decimal places
    printf("%.4lf\n", cost);

    return 0;
}

