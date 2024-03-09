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

    int padding; // real_size - num_elements

    int num_elements; // Total de elementos que vamos a procesar
    int real_size;    // Numeros de elementos total mas los que sobran o falten

    std::vector<int> imagen, imagen_local, imagen_res;

    if (rank == 0)
    {
        imagen = generar_imagen();

        num_elements = imagen.size();
        // real_size = std::ceil((double)num_elements / nprocs) * nprocs;
    }

    MPI_Bcast(&num_elements, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
        imagen.resize(num_elements);

    MPI_Bcast(imagen.data(), num_elements, MPI_INT, 0, MPI_COMM_WORLD);

    int block_size = imagen.size() / nprocs;

    imagen_local.resize(num_elements / (4 * nprocs));

    for (int i = 0; i < imagen_local.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int new_j = j < 2 ? i + (j * 4) : i + ((j - 2) * 4) + WIDTH;
            imagen_local[i] += imagen[new_j];
        }
        imagen_local[i] /= 4;
    }

    imagen_res.resize(num_elements / 4);
    MPI_Gather(imagen_local.data(), imagen_local.size(), MPI_INT, imagen_res.data(), imagen_local.size(), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
        for (int i = 1; i < 16; i++)
        {
            std::printf("%d : %d + %d + %d + %d\n", imagen_res[i], imagen[i], imagen[i + 4], imagen[i + WIDTH], imagen[i + WIDTH + 4]);
        }

    MPI_Finalize();
}