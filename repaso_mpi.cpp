#include <mpi.h>
#include <stdio.h>
#include <vector>
#include <random>

#define MAX_ELEMENTS 20
#define MIN 5
#define MAX 15

std::vector<int> generar_vector(const int n, const int min, const int max)
{
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<int> dist(min, max);

    std::vector<int> vector(n);
    for (int i = 0; i < n; i++)
    {
        vector[i] = dist(gen);
    }

    return vector;
}

std::vector<int> frecuencias(std::vector<int> vector, const int n, const int min, const int max)
{
    int distancia = max - min + 1;
    std::vector<int> frecuencias(distancia, 0);
    for (int i = 0; i < n; i++)
    {
        frecuencias[vector[i] - min] += 1;
    }
    return frecuencias;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size, padding = 0;

    std::vector<int> data;
    std::vector<int> data_local;

    if (rank == 0)
    {
        data = generar_vector(MAX_ELEMENTS, MIN, MAX);

        for (int i = 0; i < data.size(); i++)
            std::printf("%d, ", data[i]);

        int num_elements = data.size();

        int real_size = std::ceil((double)num_elements / nprocs) * nprocs;
        padding = real_size - num_elements;
        block_size = real_size / nprocs;

        data.resize(real_size, 0);

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Send(&padding, 1, MPI_INT, nprocs - 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == nprocs - 1)
            MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    data_local.resize(block_size, 0);

    MPI_Scatter(data.data(), block_size, MPI_INT, data_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    int block_size_tmp = rank == nprocs - 1 ? block_size - padding : block_size;

    auto res = frecuencias(data_local, block_size_tmp, MIN, MAX);

    std::vector<int> frecuencias_global;
    if (rank == 0)
    {
        frecuencias_global.resize(MAX - MIN + 1, 0);
    }

    MPI_Reduce(res.data(), frecuencias_global.data(), MAX - MIN + 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        for (int i = 0; i < frecuencias_global.size(); i++)
            std::printf("%2d, %d \n", i + 5, frecuencias_global[i]);

    MPI_Finalize();

    return 0;
}