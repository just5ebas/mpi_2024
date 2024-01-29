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

int sumar(int *vector, const int n)
{
    int suma = 0;
    for (int i = 0; i < n; i++)
    {
        suma = suma + vector[i];
    }
    return suma;
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

    int suma = 0;
    int suma_local = 0;
    int num_elementos = 0;

    if (rank == 0)
    {
        data = read_file();
        num_elementos = data.size();

        int real_size = std::ceil((double)num_elementos / nprocs) * nprocs;
        block_size = real_size / nprocs;
        int padding = real_size - num_elementos;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data.resize(real_size, 0);

        MPI_Scatter(data.data(), block_size, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);

        suma_local = sumar(data.data(), block_size);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        data_local.resize(block_size, 0);
        MPI_Scatter(nullptr, 0, MPI_INT, data_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        suma_local = sumar(data_local.data(), block_size);
    }

    MPI_Reduce(&suma_local, &suma, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::printf("Suma: %d\n", suma);
        double prom = (double)suma / num_elementos;
        std::printf("Promedio: %.2f\n", prom);
    }

    MPI_Finalize();
    return 0;
}*/

/*int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> data;
    std::vector<int> data_local;

    int block_size;

    int suma = 0;
    int suma_local = 0;
    int num_elementos = 0;

    if (rank == 0)
    {
        data = read_file();
        num_elementos = data.size();

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

    suma_local = sumar(data_local.data(), block_size);

    MPI_Reduce(&suma_local, &suma, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::printf("Suma: %d\n", suma);
        double prom = (double)suma / num_elementos;
        std::printf("Promedio: %.2f\n", prom);
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

    if (rank == 0)
    {
        std::vector<int> data = read_file();

        int *vector = data.data();

        int num_elementos = data.size();

        block_size = num_elementos / nprocs;
        int sobrante = num_elementos % nprocs;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        for (int i = 0; i < nprocs; i++)
        {
            int start = (block_size * i) + sobrante;
            MPI_Send(&vector[start], block_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int sumas[nprocs];

        sumas[rank] = sumar(vector, block_size + sobrante);

        for (int i = 1; i < nprocs; i++)
        {
            MPI_Recv(&sumas[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        int suma = sumar(sumas, nprocs);

        std::printf("Suma: %d\n", suma);

        double prom = (double) suma / num_elementos;

        std::printf("Promedio: %.2f\n", prom);
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int *vector_local = new int[block_size];

        MPI_Recv(vector_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int suma_local = sumar(vector_local, block_size);

        MPI_Send(&suma_local, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
