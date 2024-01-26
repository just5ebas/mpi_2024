"# mpi_2024" 

Instalar MPI
sudo apt-get update
sudo apt-get install libopenmpi-dev

Compilar codigo
mpicxx nombre.cpp -o nombre

Ejecutar codigo
mpiexec -n 4 nombre