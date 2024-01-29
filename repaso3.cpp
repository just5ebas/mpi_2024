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

/*int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> data;
    std::vector<int> data_local;

    int block_size;

    int max = 0;
    int max_local = 0;

    if (rank == 0)
    {
        data = read_file();
        int num_elementos = data.size();

        int real_size = std::ceil((double)num_elementos / nprocs) * nprocs;
        block_size = real_size / nprocs;
        int padding = real_size - num_elementos;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data.resize(real_size, data[num_elementos - 1]);

        MPI_Scatter(data.data(), block_size, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);

        max_local = maximo(data.data(), block_size);

        MPI_Send(&padding, 1, MPI_INT, nprocs - 1, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data_local.resize(block_size, 0);
        MPI_Scatter(nullptr, 0, MPI_INT, data_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        int block_size_tmp = block_size;
        if (rank == nprocs - 1)
        {
            int padding;
            MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            block_size_tmp = block_size - padding;
        }

        max_local = maximo(data_local.data(), block_size_tmp);
    }

    MPI_Reduce(&max_local, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::printf("Maximo: %d\n", max);
    }

    MPI_Finalize();
    return 0;
}*/

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> data;
    std::vector<int> data_local;

    int block_size;

    int max = 0;
    int max_local = 0;

    if (rank == 0)
    {
        data = read_file();
        int num_elementos = data.size();

        int real_size = std::ceil((double)num_elementos / nprocs) * nprocs;
        block_size = real_size / nprocs;
        int padding = real_size - num_elementos;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data.resize(real_size, 0);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    data_local.resize(block_size, 0);
    MPI_Scatter(data.data(), block_size, MPI_INT, data_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    max_local = maximo(data_local.data(), block_size);

    MPI_Reduce(&max_local, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::printf("Maximo: %d\n", max);
    }

    MPI_Finalize();
    return 0;
}