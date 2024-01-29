#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>

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

int *producto_escalar(int *vector, const int n, const int alpha)
{
    int *res = new int[n]();
    for (int i = 0; i < n; i++)
    {
        res[i] = alpha * vector[i];
    }
    return res;
}

void producto_escalar_m(int *vector, const int n, const int alpha, int *res)
{
    for (int i = 0; i < n; i++)
    {
        res[i] = alpha * vector[i];
    }
}

/*int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size;

    if (rank == 0)
    {
        std::vector<int> data = read_file();
        int *vector = data.data();

        block_size = data.size() / nprocs;
        int sobrante = data.size() % nprocs;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        for (int i = 1; i < nprocs; i++)
        {
            int start = (block_size * i) + sobrante;
            MPI_Send(&vector[start], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int **productos_parciales = new int *[nprocs];
        productos_parciales[rank] = producto_escalar(vector, block_size + sobrante, 5);

        for (int i = 1; i < nprocs; i++)
        {
            int *prod = new int[block_size];
            MPI_Recv(prod, block_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            productos_parciales[i] = prod;
        }

        int block_size_aux;
        for (int i = 0; i < nprocs; i++)
        {
            if (i == 0)
                block_size_aux = block_size + sobrante;
            else
                block_size_aux = block_size;
            for (int j = 0; j < block_size_aux; j++)
                std::printf("%d, ", productos_parciales[i][j]);
        }
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int *vector_local = new int[block_size]();

        MPI_Recv(vector_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        vector_local = producto_escalar(vector_local, block_size, 5);

        MPI_Send(&vector_local[0], block_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
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

    int block_size;
    int padding = 0;

    std::vector<int> data;

    if (rank == 0)
    {
        data = read_file();

        int num_elementos = data.size();
        int real_size = std::ceil((double)num_elementos / nprocs) * nprocs;
        block_size = real_size / nprocs;
        padding = real_size - num_elementos;

        data.resize(real_size, 0);
        int *vector = data.data();

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Send(&padding, 1, MPI_INT, nprocs - 1, 0, MPI_COMM_WORLD);

        MPI_Scatter(vector, block_size, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);

        std::vector<int> producto_escalar_total(real_size, 0);

        producto_escalar_m(vector, block_size, 5, producto_escalar_total.data());

        MPI_Gather(MPI_IN_PLACE, 0, MPI_INT, producto_escalar_total.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        producto_escalar_total.resize(num_elementos);

        for (int i = 0; i < num_elementos; i++)
        {
            std::printf("%d, ", producto_escalar_total[i]);
        }
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        int *vector_local = new int[block_size]();

        if (rank == nprocs - 1)
            MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Scatter(nullptr, 0, MPI_INT, vector_local, block_size, MPI_INT, 0, MPI_COMM_WORLD);

        vector_local = producto_escalar(vector_local, block_size - padding, 5);

        MPI_Gather(vector_local, block_size, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}