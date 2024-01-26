#include <iostream>
#include <vector>
#include <mpi.h>
#include <stdio.h>
#include <string>
#include <cmath>

#define MATRIX_DIMENSION 25

void matrix_mult(double *A, double *b, double *c, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        double tmp = 0;
        for (int j = 0; j < cols; j++) {
            tmp = tmp + A[i * cols + j] * b[j];
        }
        c[i] = tmp;
    }
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int rows_per_rank;
    int rows_alloc = MATRIX_DIMENSION;
    int padding = 0;

    if (MATRIX_DIMENSION % nprocs != 0) {
        rows_alloc = std::ceil((double) MATRIX_DIMENSION / nprocs) * nprocs;
        padding = rows_alloc - MATRIX_DIMENSION;
    }

    rows_per_rank = rows_alloc / nprocs;

    std::vector<double> A(MATRIX_DIMENSION * rows_alloc);
    std::vector<double> b(MATRIX_DIMENSION);
    std::vector<double> c(rows_alloc);

    if (rank == 0) {
        std::printf("Dimension: %d, rows_allocs: %d, rows_per_rank: %d, padding: %d\n",
                    MATRIX_DIMENSION, rows_alloc, rows_per_rank, padding);

        for (int i = 0; i < MATRIX_DIMENSION; i++) {
            for (int j = 0; j < MATRIX_DIMENSION; j++) {
                int index = i * MATRIX_DIMENSION + j;
                A[index] = i;
            }
        }
        for (int i = 0; i < MATRIX_DIMENSION; i++) b[i] = 1;
    }

    std::vector<double> A_local(MATRIX_DIMENSION * rows_per_rank);
    std::vector<double> c_local(rows_per_rank);

    MPI_Scatter(A.data(), rows_per_rank * MATRIX_DIMENSION, MPI_DOUBLE, // datos del envio
                A_local.data(), rows_per_rank * MATRIX_DIMENSION,
                MPI_DOUBLE,                            // datos de recepcion
                0, MPI_COMM_WORLD);                                     // root & comm (coordina)

    MPI_Bcast(b.data(), MATRIX_DIMENSION, MPI_DOUBLE,
              0, MPI_COMM_WORLD);

    matrix_mult(A_local.data(), b.data(), c_local.data(), rows_per_rank, MATRIX_DIMENSION);

    MPI_Gather(c_local.data(), rows_per_rank, MPI_DOUBLE,
               c.data(), rows_per_rank, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        c.resize(MATRIX_DIMENSION);

        // Imprimir el resultado
        std::printf("Resultado: \n");
        for (int i = 0; i < MATRIX_DIMENSION; i++) {
            std::printf("%.0f, ", c[i]);
        }

        std::printf("\n");
    }

    MPI_Finalize();

    return 0;
}