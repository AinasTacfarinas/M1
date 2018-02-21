/*Thread principal va faire un init thread (avec argument serialized)
-> création de plusieurs threads "senders" qui vont faire un start server, un Ssend puis se mettre en attente sur la condition (attend que le serveur ait reçu le message)
Start_server va donc créer le thread via pthread_create et lire le message reçu puis mettre a jour la condition pour reveiller son "sender"
*/
#include "mpi_server.h"

static server the_server;

//ICI CODE THREAD SERVEUR A COMPLETER
void *server_reception(int tag, int source){
  int flag;
  MPI_STATUS status;
  char message[SIZE];
  
  MPI_Iprobe(source, tag, COMM_WORLD, &flag, &status);

  if(flag!=0){
    MPI_Recv(message,SIZE, MPI_CHAR, status.MPI_SOURCE);
    
  }
  return NULL;
}


void start_server(void (*callback)(int tag, int source)){
  //A COMPLETER
  
  if(pthread 
}

void destroy_server(){
  //A COMPLETER
}

pthread_mutex_t* getMutex(){
  // A COMPLETER
  return the_server.mutex;
}
