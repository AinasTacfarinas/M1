#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]){

  int rang;
  int size;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  printf("Processus %d sur %d procs : Hello\n", rang, size);

  MPI_Finalize();
  return 0;
}
