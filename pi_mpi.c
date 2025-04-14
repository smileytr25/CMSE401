#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

static long num_steps = 100000;
double step;

int main(int argc, char *argv[])
{
    int i, id, num_procs;
    double x, pi, local_sum = 0.0, global_sum = 0.0;

    step = 1.0 / (double) num_steps;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);       // get this process's ID
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // get total number of processes

    // Each process computes its partial sum
    for (i = id; i < num_steps; i += num_procs) {
        x = (i + 0.5) * step;
        local_sum += 4.0 / (1.0 + x * x);
    }

    // Reduce all partial sums into the global sum on process 0
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (id == 0) {
        pi = global_sum * step;
        printf("Computed PI = %.15f\n", pi);
    }

    MPI_Finalize();
    return 0;
}
