#include <stdio.h>
#include <string.h>
#include <mpi.h>
#define MASTER 0
#define SIZE 128

int main(int argc, char **argv){
  int my_rank;
  int nb_proc;
  int source;
  int dest;
  int provided;
  int tag =0;
  char message[SIZE];
  MPI_Status status;
  
  
  MPI_Init(&argc, &argv);
  /*MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided);*/
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

  if(my_rank == 0){

    sprintf(message, "Hello Neighbor %d", my_rank);
    dest = ( (my_rank+1)%nb_proc );
    MPI_Ssend(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);

    source = ( (my_rank-1)%nb_proc );
    MPI_Recv(message, SIZE, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
    printf("Je suis %d et message de %d est : %s \n", my_rank, status.MPI_SOURCE, message);
    
  }

  else{

    source = ( (my_rank-1)%nb_proc );
    MPI_Recv(message, SIZE, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
    printf("Je suis %d et message de %d est : %s \n", my_rank, status.MPI_SOURCE, message);
    
    sprintf(message, "Hello Neighbor %d", my_rank);
    dest = ( (my_rank+1)%nb_proc );
    MPI_Ssend(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);

  }

  MPI_Finalize();

  return 0;

}
