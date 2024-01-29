#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::vector<int> read_file()
{
    std::fstream fs("./datos.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;
    while (std::getline(fs, line))
    {
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

int *obtener_frecuencias(int *vector, const int n)
{
    int *conteo = new int[101]();
    for (int i = 0; i < n; i++)
        conteo[vector[i]] += 1;

    return conteo;
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size;
    int padding;
    std::vector<int> data;
    std::vector<int> data_local;

    if (rank == 0)
    {
        data = read_file();

        int num_elementos = data.size();
        int real_size = std::ceil((double)data.size() / nprocs) * nprocs;
        block_size = real_size / nprocs;
        padding = real_size - num_elementos;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data.resize(real_size, 0);

        MPI_Send(&padding, 1, MPI_INT, nprocs - 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == nprocs - 1)
        {
            MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    data_local.resize(block_size);
    MPI_Scatter(data.data(), block_size, MPI_INT, data_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    int block_size_tmp = block_size;
    if (rank == nprocs - 1)
        block_size_tmp = block_size - padding;
    int *conteo_local = obtener_frecuencias(data_local.data(), block_size_tmp);

    int *conteo_global = new int[101];
    MPI_Reduce(conteo_local, conteo_global, 101, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        for (int i = 0; i < 101; i++)
        {
            std::printf("%3d, %d\n", i, conteo_global[i]);
        }

    MPI_Finalize();

    return 0;
}