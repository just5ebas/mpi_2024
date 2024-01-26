#include <mpi.h>
#include <stdio.h>  // Incluimos la biblioteca para printf
#include <string>
#include <vector>
#include <math.h>

#define MAX_ITEMS 25

int sumar(int *tmp, int n) {
    int suma = 0;
    for (int i = 0; i < n; i++) {
        suma += tmp[i];
    }
    return suma;
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size;
    int real_size;
    int padding;

    if (MAX_ITEMS % nprocs != 0) {
        real_size = std::ceil((double) MAX_ITEMS / nprocs) * nprocs;
        block_size = real_size / nprocs;
        padding = real_size - MAX_ITEMS;
    }

    std::vector<int> data_local(block_size); // todos los ranks
    int suma_total = 0;
    int suma_parcial = 0;

    if (rank == 0) {
        // Inicializacion del vector
        std::vector<int> data(real_size);

        std::printf("Dimension: %d, real_size: %d, block_size: %d, padding: %d\n",
                    MAX_ITEMS, real_size, block_size, padding);

        for (int i = 0; i < MAX_ITEMS; i++) {
            data[i] = i;
        }

        // Enviar los datos
        MPI_Scatter(data.data(), block_size, MPI_INT,
                    MPI_IN_PLACE, 0, MPI_INT,
                    0, MPI_COMM_WORLD);

        suma_parcial = sumar(data.data(), block_size);
    } else {
        // Recibir datos
        MPI_Scatter(nullptr, 0, MPI_INT,
                    data_local.data(), block_size, MPI_INT,
                    0, MPI_COMM_WORLD);

        if (rank == nprocs - 1) {
            block_size = block_size - padding;
        }

        suma_parcial = sumar(data_local.data(), block_size);
    }

    MPI_Reduce(&suma_parcial, &suma_total, 1, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        std::printf("Resultado: %d\n", suma_total);

    MPI_Finalize();

    return 0;
}