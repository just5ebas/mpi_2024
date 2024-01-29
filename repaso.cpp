#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

std::vector<int> read_file()
{
    std::fstream fs("./datos2.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;
    while (std::getline(fs, line))
    {
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

int *obtener_frecuencias(std::vector<int> vector, const int n)
{
    std::vector<int> conteo(101, 0);
    for (int i = 0; i < n; i++)
        conteo[vector[i]] += 1;

    return conteo.data();
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
            int start = (block_size * i) + sobrante;
            std::printf("start = %d, block_size = %d\n", start, block_size);
            MPI_Send(&block_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&vector[start], block_size, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        std::vector<int *> conteos(4);

        conteos[rank] = obtener_frecuencias(data, block_size + sobrante);

        for (int i = 1; i < nprocs; i++)
        {
            int *conteo = new int[block_size];
            MPI_Recv(&conteo, 101, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            conteos[i] = conteo;
        }

        int *frecuencias = new int[100];

        for (int i = 0; i < 101; i++)
        {
            frecuencias[i] = conteos[0][i] + conteos[1][i] + conteos[2][i] + conteos[3][i];
        }
    }
    else
    {
        int block_size;
        MPI_Recv(&block_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::printf("%d", block_size);

        std::vector<int> data_local(block_size, 0);
        MPI_Recv(data_local.data(), block_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *conteo_local = new int(101);
        conteo_local = obtener_frecuencias(data_local, block_size);

        MPI_Send(&conteo_local[0], 101, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}