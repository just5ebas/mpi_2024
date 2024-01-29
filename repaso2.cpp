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

int maximo(int *vector, const int n)
{
    int max = 0;
    for (int i = 0; i < n; i++)
    {
        if (vector[i] > max)
            max = vector[i];
    }
    return max;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (rank == 0)
    {
        std::vector<int> data = read_file();
        int *vector = data.data();

        int block_size = data.size() / nprocs;
        int sobrante = data.size() % nprocs;

        for (int i = 1; i < nprocs; i++)
        {
            MPI_Send(&block_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            int start = (i * block_size) + sobrante;
            MPI_Send(&vector[start], block_size, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        int maximos[nprocs];

        maximos[0] = maximo(vector, block_size + sobrante);

        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(&maximos[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int max = maximo(maximos, 4);

        std::printf("Maximo: %d\n", max);
    }
    else
    {
        int block_size;
        MPI_Recv(&block_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // std::printf("Size: %d\n", block_size);

        int *vector_local = new int[block_size];
        MPI_Recv(vector_local, block_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int maximo_local = maximo(vector_local, block_size);

        // std::printf("MAX: %d\n", maximo_local);

        MPI_Send(&maximo_local, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}