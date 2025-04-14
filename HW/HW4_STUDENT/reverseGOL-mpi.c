#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

#define max_nodes 264
#define str_length 50

int fitness(char * plate1, char * plate2, int n){
    int errors = 0;
    for(int i = 1; i <= n; i++)
        for(int j = 1; j <= n; j++) {
            int index = i * (n + 2) + j;
            errors += !(plate1[index] == plate2[index]);
        }
    return errors;
}

int live(int index, char * plate[2], int which, int n){
    return (plate[which][index - n - 3]
          + plate[which][index - n - 2]
          + plate[which][index - n - 1]
          + plate[which][index - 1]
          + plate[which][index + 1]
          + plate[which][index + n + 1]
          + plate[which][index + n + 2]
          + plate[which][index + n + 3]);
}

int iteration(char * plate[2], int which, int n){
    for(int i = 1; i <= n; i++)
        for(int j = 1; j <= n; j++) {
            int index = i * (n + 2) + j;
            int num = live(index, plate, which, n);
            plate[!which][index] = (plate[which][index]) ? (num == 2 || num == 3) : (num == 3);
        }
    return !which;
}

void print_plate(char * plate, int n){
    if (n < 60) {
        for(int i = 1; i <= n; i++){
            for(int j = 1; j <= n; j++)
                printf("%d", (int) plate[i * (n + 2) + j]);
            printf("\n");
        }
    } else {
        printf("Plate too large to print to screen\n");
    }
}

void makerandom(char * plate, int n) {
    for(int i = 1; i <= n; i++)
        for(int j = 0; j < n; j++)
            plate[i * (n+2) + j + 1] = (rand() % 100) > 10;
}

void mutate(char * plate, char * best_plate, int n, int rate) {
    for(int i = 1; i <= n; i++)
        for(int j = 0; j < n; j++) {
            int index = i * (n+2) + j + 1;
            plate[index] = ((rand() % 100) < rate) ? (rand() % 2) : best_plate[index];
        }
}

void cross(char * plate1, char * plate2, int n) {
    int start = 0;
    int end = (n+2)*(n+2);
    int crosspoint = rand() % end;
    if (crosspoint < end/2) {
        end = crosspoint;
    } else {
        start = crosspoint;
    }
    for(int i = start; i <= end; i++)
        plate1[i] = plate2[i];
}

char * readplate(char * filename, int *n) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int N, M;
    if(fscanf(fp, "%d %d", &N, &M) != 2){
        printf("File not correct format\n");
        exit(EXIT_FAILURE);
    }
    *n = N;
    char *plate = (char *) calloc((N+2)*(N*2), sizeof(char));
    char line[N];
    for(int i = 1; i <= N; i++){
        fscanf(fp, "%s", line);
        for(int j = 0; j < N; j++)
            plate[i * (N + 2) + j + 1] = (line[j] == '1');
    }
    fclose(fp);
    return plate;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int which = 0, n, npop = 10000, ngen = 1000, M = 0;
    int rand_seed = (argc > 2) ? (atoi(argv[2]) + 1) * 7 : time(NULL);
    srand(rand_seed);

    char *target_plate = readplate(argv[1], &n);
    char *buffer_plate = (char *) calloc((n+2)*(n+2), sizeof(char));

    char *population[npop];
    int pop_fitness[npop];
    int best = 0, sbest = 1;
    int plateSize = (n + 2) * (n + 2);

    for(int i = 0; i < npop; i++) {
        pop_fitness[i] = n * n;
        population[i] = (char *) calloc(plateSize, sizeof(char));
        if (i < npop/2)
            mutate(population[i], target_plate, n, 10);
        else
            makerandom(population[i], n);
    }

    for(int g = 0; g < ngen; g++) {
        for(int i = 0; i < npop; i++) {
            char *plate[2] = { population[i], buffer_plate };
            iteration(plate, 0, n);
            pop_fitness[i] = fitness(buffer_plate, target_plate, n);

            if (pop_fitness[i] < pop_fitness[best]) {
                sbest = best;
                best = i;
                if (pop_fitness[best] == 0) {
                    char *temp = target_plate;
                    target_plate = population[best];
                    population[best] = temp;
                    printf("Perfect match at Rank %d!\n", rank);
                    print_plate(target_plate, n);
                    pop_fitness[best] = n * n;
                    M++;
                }
            } else if (sbest == best) {
                sbest = i;
            }
        }

        int rate = (int)((double)pop_fitness[best] / (n*n) * 100);
        for(int i = 0; i < npop; i++) {
            if (i == sbest) cross(population[i], population[best], n);
            else if (i != best) {
                if (i < npop/3)
                    mutate(population[i], population[best], n, rate);
                else if (i < (npop*2)/3)
                    cross(population[i], population[best], n);
                else
                    makerandom(population[i], n);
            }
        }
    }

    // Neighbor exchange
    int localBestFitness = pop_fitness[best];
    char *localBestPlate = population[best];
    int next = (rank + 1) % size;
    int prev = (rank == 0) ? size - 1 : rank - 1;

    int neighborFitness;
    char *neighborPlate = (char *)malloc(plateSize);

    if (rank == 0) {
        MPI_Recv(&neighborFitness, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(neighborPlate, plateSize, MPI_CHAR, prev, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Send(&localBestFitness, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        MPI_Send(localBestPlate, plateSize, MPI_CHAR, next, 1, MPI_COMM_WORLD);
    } else {
        MPI_Send(&localBestFitness, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        MPI_Send(localBestPlate, plateSize, MPI_CHAR, next, 1, MPI_COMM_WORLD);

        MPI_Recv(&neighborFitness, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(neighborPlate, plateSize, MPI_CHAR, prev, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // If neighbor was better, adopt it
    if (neighborFitness < localBestFitness) {
        memcpy(population[best], neighborPlate, plateSize);
        localBestFitness = neighborFitness;
    }

    free(neighborPlate);

    // Final gathering to rank 0
    if (rank != 0) {
        MPI_Send(&localBestFitness, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(population[best], plateSize, MPI_CHAR, 0, 3, MPI_COMM_WORLD);
    } else {
        int globalBestFitness = localBestFitness;
        char *globalBestPlate = (char *)malloc(plateSize);
        memcpy(globalBestPlate, population[best], plateSize);

        for (int i = 1; i < size; i++) {
            int recvFitness;
            char *recvPlate = (char *)malloc(plateSize);
            MPI_Recv(&recvFitness, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(recvPlate, plateSize, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (recvFitness < globalBestFitness) {
                globalBestFitness = recvFitness;
                memcpy(globalBestPlate, recvPlate, plateSize);
            }
            free(recvPlate);
        }

        printf("\n=== Overall Best Solution ===\n");
        print_plate(globalBestPlate, n);
        printf("Final Result Fitness = %d over %d generations\n", globalBestFitness, ngen);

        FILE *fp = fopen("revGOL_solution.txt", "w");
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++)
                fprintf(fp, "%d", globalBestPlate[i * (n + 2) + j]);
            fprintf(fp, "\n");
        }
        fclose(fp);
        printf("✅ Saved to revGOL_solution.txt\n");
        free(globalBestPlate);
    }

    free(target_plate);
    free(buffer_plate);
    for(int i = 0; i < npop; i++)
        free(population[i]);

    MPI_Finalize();
    return 0;
}

