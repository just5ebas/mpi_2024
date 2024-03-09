#include <mpi.h>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <random>

#define WIDTH 720
#define HEIGHT 480

std::vector<int> generar_imagen()
{
    std::vector<int> imagen(WIDTH * HEIGHT * 4);

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    for (int i = 0; i < WIDTH * HEIGHT * 4; i++)
    {
        imagen[i] = dist(gen);
    }

    return imagen;
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // Numero de proceso o de rank [ID]
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // Cuantos procesos hay      [TOTAL]

    if (rank == 0)
    {
        auto imagen = generar_imagen();

        int num_elements = imagen.size();

        int block_size = num_elements / nprocs;
        int sobrante = num_elements % nprocs;

        int *v = imagen.data();

        for (int i = 1; i <= nprocs - 1; i++)
        {
            MPI_Send(&block_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            int start = (block_size * i) + sobrante;
            MPI_Send(&v[start], block_size, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        std::vector<int> imagen_res(num_elements);
        for (int i = 0; i < block_size; i++)
        {
            if (i % 4 == 0)
                imagen_res[i] = 0;
            else
                imagen_res[i] = imagen[i];
        }

        for (int i = 1; i <= nprocs - 1; i++)
        {
            int *aux = new int[block_size];
            MPI_Recv(aux, block_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j < block_size; j++)
            {
                imagen_res[sobrante + (block_size * i) + j] = aux[j];
            }
        }

        for (int i = 0; i < 20; i++)
        {
            std::printf("%d, ", imagen_res[i]);
        }
    }
    else
    {
        int block_size;
        MPI_Recv(&block_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::printf("Recibiendo blocksize %d\n", block_size);

        int *imagen_local = new int[block_size](); // [0,0,0,0,0,0,0]
        MPI_Recv(imagen_local, block_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < block_size; i++)
        {
            if (i % 4 == 0)
                imagen_local[i] = 0;
        }

        MPI_Send(imagen_local, block_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}