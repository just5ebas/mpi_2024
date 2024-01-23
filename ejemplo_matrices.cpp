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

    if (rank == 0) {
        std::printf("Dimension: %d, rows_allocs: %d, rows_per_rank: %d, padding: %d\n",
                    MATRIX_DIMENSION, rows_alloc, rows_per_rank, padding);

        std::vector<double> A(MATRIX_DIMENSION * rows_alloc);
        std::vector<double> b(MATRIX_DIMENSION);
        std::vector<double> c(rows_alloc);

        for (int i = 0; i < MATRIX_DIMENSION; i++) {
            for (int j = 0; j < MATRIX_DIMENSION; j++) {
                int index = i * MATRIX_DIMENSION + j;
                A[index] = i;
            }
        }
        for (int i = 0; i < MATRIX_DIMENSION; i++) b[i] = 1;

        // Enviar la matriz A
        MPI_Scatter(A.data(), rows_per_rank * MATRIX_DIMENSION, MPI_DOUBLE, // datos del envio
                    MPI_IN_PLACE, 0, MPI_DOUBLE,                            // datos de recepcion
                    0, MPI_COMM_WORLD);                                     // root & comm (coordina)

        // Enviar el vector B
        MPI_Bcast(b.data(), MATRIX_DIMENSION, MPI_DOUBLE,
                  0, MPI_COMM_WORLD);

        // realizar calculo: c = A * b
        matrix_mult(A.data(), b.data(), c.data(), rows_per_rank, MATRIX_DIMENSION);

        // Recibir resultados parciales de los otros ranks
        MPI_Gather(MPI_IN_PLACE, 0,
                   MPI_DOUBLE, // Este rank no se envia a si mismo, pero tiene el calculo almacenado, por ello se pone IN_PLACE
                   c.data(), rows_per_rank, MPI_DOUBLE,
                   0, MPI_COMM_WORLD);

        c.resize(MATRIX_DIMENSION);

        // Imprimir el resultado
        std::printf("Resultado: \n");
        for (int i = 0; i < MATRIX_DIMENSION; i++) {
            std::printf("%.0f, ", c[i]);
        }

        std::printf("\n");


    } else {
        std::vector<double> A_local(MATRIX_DIMENSION * rows_per_rank);
        std::vector<double> b_local(MATRIX_DIMENSION);
        std::vector<double> c_local(rows_per_rank);

        // Recibir Matriz A
        MPI_Scatter(nullptr, 0, MPI_DOUBLE, // Desde aqui no enviamos, por eso estos parametros
                    A_local.data(), MATRIX_DIMENSION * rows_per_rank,
                    MPI_DOUBLE, // Aqui si recibimos, por ello lo ponemos de esta forma
                    0, MPI_COMM_WORLD); // Estos parametros indican que rank envia

        std::printf("RANK_%d: [%.0f, %.0f]\n", rank, A_local[0], A_local.back());

        // Recibir Vector b
        MPI_Bcast(b_local.data(), MATRIX_DIMENSION, MPI_DOUBLE,
                  0, MPI_COMM_WORLD);

        int rows_per_rank_tmp = rows_per_rank;
        if (rank == nprocs - 1) {
            rows_per_rank_tmp = MATRIX_DIMENSION - rank * rows_per_rank;
            // Tambien podemos hacer la operacion con el padding:
            // rows_per_rank_tmp = rows_per_rank - padding
        }

        // realizar calculo: c = A * b -> Tener cuidado con el ultimo rank
        matrix_mult(A_local.data(), b_local.data(), c_local.data(), rows_per_rank_tmp, MATRIX_DIMENSION);

        // Enviar el resultado al rank 0
        MPI_Gather(c_local.data(), rows_per_rank, MPI_DOUBLE,
                   nullptr, 0, MPI_DOUBLE, // Estos parametros debido a que no recibimos nada en estos ranks
                   0, MPI_COMM_WORLD);

    }

    MPI_Finalize();

    return 0;
}