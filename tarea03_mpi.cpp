#include <mpi.h>
#include <stdio.h>  // Incluimos la biblioteca para printf
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

std::vector<int> read_file() {
    std::fstream fs("./datos2.txt", std::ios::in);
    std::string line;

    std::vector<int> ret;

    while (std::getline(fs, line)) {
        ret.push_back(std::stoi(line));
    }
    fs.close();
    return ret;
}

int sumar(int *tmp, int n) {
    int suma = 0;
    for (int i = 0; i < n; i++) {
        suma += tmp[i];
    }
    return suma;
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (rank == 0) {
        std::printf("Total de ranks: %d\n", nprocs);

        std::vector<int> datos = read_file();

        int size = datos.size();
        int count = std::ceil(size / nprocs);

        std::printf("Size: %d, Count: %d\n", size, count);

        int *data = datos.data();

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            int start = (rank_id - 1) * count;
            MPI_Send(&data[start], count, MPI_INT, rank_id, 0, MPI_COMM_WORLD);
        }

        int suma_ranks[nprocs];

        suma_ranks[0] = sumar(data + ((nprocs - 1) * count), size - ((nprocs - 1) * count));

        std::printf("%d", suma_ranks[0]);

        for (int rank_id = 1; rank_id < nprocs; rank_id++) {
            MPI_Recv(&suma_ranks[rank_id], 1, MPI_INT, rank_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        std::string str = "";
        for (int i = 0; i < nprocs; i++) {
            str = str + std::to_string(suma_ranks[i]) + ", ";
        }
        std::printf("Sumas parciales: %s\n", str.c_str());

        int sumaTotal = sumar(suma_ranks, nprocs);

        std::printf("Suma total: %d\n", sumaTotal);

    } else {
        MPI_Status status;
        int count;

        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        int *data = new int[count];

        MPI_Recv(data, count, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int suma_parcial = sumar(data, count);

        MPI_Send(&suma_parcial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}