#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, mprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mprocs);

    std::printf("rank %d de %d\n", rank, mprocs);

    MPI_Finalize();

    return 0;
}
