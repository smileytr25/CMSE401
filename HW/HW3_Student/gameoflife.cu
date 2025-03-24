#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <assert.h>
#include <cuda.h>
#include "png_util.h"

#define MAX_N 20000

char plate[2][(MAX_N + 2) * (MAX_N + 2)];
int which = 0;
int n;

__global__ void updatePlate(char *plate, int n, int which) {
    int i = blockIdx.y * blockDim.y + threadIdx.y + 1;
    int j = blockIdx.x * blockDim.x + threadIdx.x + 1;

    if (i > n || j > n) return;

    int idx = i * (n + 2) + j;
    int base = which * (n + 2) * (n + 2);
    int next = (!which) * (n + 2) * (n + 2);

    int num =
        plate[base + idx - n - 3] +
        plate[base + idx - n - 2] +
        plate[base + idx - n - 1] +
        plate[base + idx - 1] +
        plate[base + idx + 1] +
        plate[base + idx + n + 1] +
        plate[base + idx + n + 2] +
        plate[base + idx + n + 3];

    if (plate[base + idx]) {
        plate[next + idx] = (num == 2 || num == 3) ? 1 : 0;
    } else {
        plate[next + idx] = (num == 3);
    }
}

void print_plate() {
    if (n < 60) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                printf("%d", (int)plate[which][i * (n + 2) + j]);
            }
            printf("\n");
        }
    } else {
        printf("Plate too large to print to screen\n");
    }
    printf("\0");
}

void plate2png(char *filename) {
    unsigned char *img = (unsigned char *)malloc(n * n * sizeof(unsigned char));
    image_size_t sz;
    sz.width = n;
    sz.height = n;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            int pindex = i * (n + 2) + j;
            int index = (i - 1) * n + j - 1;
            img[index] = plate[which][pindex] > 0 ? 255 : 0;
        }
    }

    printf("Writing file\n");
    write_png_file(filename, img, sz);
    printf("done writing png\n");
    free(img);
    printf("done freeing memory\n");
}

int main() {
    int M;
    char line[MAX_N];

    if (scanf("%d %d", &n, &M) == 2) {
        size_t platesize = 2 * (n + 2) * (n + 2) * sizeof(char);
        memset(plate[0], 0, platesize / 2);
        memset(plate[1], 0, platesize / 2);

        if (n > 0) {
            for (int i = 1; i <= n; i++) {
                scanf("%s", line);
                for (int j = 0; j < n; j++) {
                    plate[0][i * (n + 2) + j + 1] = line[j] - '0';
                }
            }
        } else {
            n = MAX_N;
            for (int i = 1; i <= n; i++)
                for (int j = 0; j < n; j++)
                    plate[0][i * (n + 2) + j + 1] = rand() % 2;
        }

        // CUDA setup
        char *d_plate;
        cudaMalloc(&d_plate, platesize);
        cudaMemcpy(d_plate, plate, platesize, cudaMemcpyHostToDevice);

        dim3 threadsPerBlock(16, 16);
        dim3 numBlocks((n + 15) / 16, (n + 15) / 16);

        for (int i = 0; i < M; i++) {
            printf("\nIteration %d:\n", i);
            updatePlate<<<numBlocks, threadsPerBlock>>>(d_plate, n, which);
            cudaDeviceSynchronize();
            which = !which;
        }

        // Copy back to CPU
        cudaMemcpy(plate, d_plate, platesize, cudaMemcpyDeviceToHost);

        printf("\n\nFinal:\n");
        plate2png("plate.png");
        print_plate();

        // Free device memory
        cudaFree(d_plate);
    }

    return 0;
}

