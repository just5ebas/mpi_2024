#include <mpi.h>
#include <stdio.h> // Biblioteca para printf

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int data[100];

    if (rank == 0) {
        std::printf("Numero de ranks: %d\n", nprocs);

        for (int i = 0; i < 100; i++) {
            data[i] = i;
        }

        for (int i = 1; i < nprocs; i++) {
            std::printf("Enviando datos al rank: %d\n", i);

            MPI_Send(data,      // datos
                     100,       // count
                     MPI_INT,   // tipo de dato
                     i,         // rank destino
                     0,         // TAG
                     MPI_COMM_WORLD // Grupo
            );
        }

    } else {
        std::printf("Recibiendo datos en el rank: %d\n", rank);

        MPI_Recv(data,          // datos
                 100,           // count
                 MPI_INT,       // tipo de dato
                 0,             // rank origen
                 0,             // TAG
                 MPI_COMM_WORLD,// Grupo
                 MPI_STATUS_IGNORE// estado/status
        );

        std::string str = "";

        for (int i = 0; i < 10; i++) {
            str = str + std::to_string(data[i]) + ",";
        }

        std::printf("RANK_%d recibio los datos ==> %s\n", rank, str.c_str());

    }

    MPI_Finalize();

    return 0;
}
