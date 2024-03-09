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

    int block_size; // Al numero de elementos que va a procesar cada rank o proceso
    int padding;    // real_size - num_elements

    int num_elements; // Total de elementos que vamos a procesar
    int real_size;    // Numeros de elementos total mas los que sobran o falten

    if (rank == 0)
    {
        std::vector<int> imagen = generar_imagen();

        num_elements = imagen.size();
        real_size = std::ceil((double)num_elements / nprocs) * nprocs;

        padding = real_size - num_elements;
        block_size = real_size / nprocs;

        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD); // Que enviamos

        MPI_Send(&padding, 1, MPI_INT, nprocs - 1, 0, MPI_COMM_WORLD);

        std::printf("Enviando blocksize %d\n", block_size);

        MPI_Scatter(imagen.data(), block_size, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);
        
        std::vector<int> imagen_res(real_size);

        for (int i = 0; i < block_size; i++)
        {
            if (i % 4 == 0)
                imagen_res[i] = 0;
            else
                imagen_res[i] = imagen[i];
        }

        MPI_Gather(MPI_IN_PLACE, 0, MPI_INT, imagen_res.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        imagen_res.resize(num_elements);

        for (int i = 0; i < 20; i++)
        {
            std::printf("%d, ", imagen_res[i]);
        }
    }
    else
    {
        MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD); // Que recibimos

        std::printf("Recibiendo blocksize %d\n", block_size);

        if (rank == nprocs - 1)
        {
            MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::vector<int> imagen_local(block_size);

        MPI_Scatter(nullptr, 0, MPI_INT, imagen_local.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

        int block_size_tmp = rank == nprocs - 1 ? block_size - padding : block_size;
        for (int i = 0; i < block_size_tmp; i++)
        {
            if (i % 4 == 0)
            {
                imagen_local[i] = 0;
            }
        }

        MPI_Gather(imagen_local.data(), block_size, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}