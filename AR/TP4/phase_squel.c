#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGINIT 0
#define NB_SITE 6

#define DIAMETRE 5

void simulateur(void) {
   int i;

   /* nb_voisins_in[i] est le nombre de voisins entrants du site i */
   /* nb_voisins_out[i] est le nombre de voisins sortants du site i */
   int nb_voisins_in[NB_SITE+1] = {-1, 2, 1, 1, 2, 1, 1};
   int nb_voisins_out[NB_SITE+1] = {-1, 2, 1, 1, 1, 2, 1};

   int min_local[NB_SITE+1] = {-1, 4, 7, 1, 6, 2, 9};

   /* liste des voisins entrants */
   int voisins_in[NB_SITE+1][2] = {{-1, -1},
				{4, 5}, {1, -1}, {1, -1},
				{3, 5}, {6, -1}, {2, -1}};
                               
   /* liste des voisins sortants */
   int voisins_out[NB_SITE+1][2] = {{-1, -1},
				{2, 3}, {6, -1}, {4, -1},
				{1, -1}, {1, 4}, {5,-1}};

   for(i=1; i<=NB_SITE; i++){
      MPI_Send(&nb_voisins_in[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&nb_voisins_out[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins_in[i], nb_voisins_in[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(voisins_out[i], nb_voisins_out[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
      MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
   }
}

void calcul_min(int rang) {
  int nb_voisins_in, nb_voisins_out;

  int min_local, s_count = 0;
  MPI_Status status;
  int i,j,condD,condS,tmp_min;
  
  /* Initialisation. */
  MPI_Recv(&nb_voisins_in, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&nb_voisins_out, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  int nb_v_in[nb_voisins_in];
  int nb_v_out[nb_voisins_out];
  MPI_Recv(nb_v_in, nb_voisins_in, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(nb_v_out, nb_voisins_out, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  printf("Mon rang %d Nb in %d Nb out %d min local %d\n", rang, nb_voisins_in, nb_voisins_out,min_local);
  int r_count[nb_voisins_in];
  for(i=0;i<nb_voisins_in;i++)
    r_count[i]=0;
  do {
    condD=0;
    condS = 1;
	if( s_count>=DIAMETRE)
	condS=0;
      for(i=0;i<nb_voisins_in;i++)
        if(r_count[i]<s_count)
          condS=0;
    while(condS){ 
      for(i=0;i<nb_voisins_out;i++) {
        printf("Rang %d envoie %d à %d\n",rang,min_local,nb_v_out[i]);
        MPI_Send(&min_local, 1, MPI_INT, nb_v_out[i], TAGINIT, MPI_COMM_WORLD);
      }
      s_count++;
      condS=1;
	if( s_count>=DIAMETRE){
		condS=0;
		continue;
	}
      for(i=0;i<nb_voisins_in;i++)
        if(r_count[i]<s_count){
          condS=0;
		break;
	}
    }
    MPI_Recv(&tmp_min, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    printf("Rang %d recoit %d de %d\n", rang, tmp_min, status.MPI_SOURCE);
	
    for(i=0;nb_v_in[i]!=status.MPI_SOURCE;i++);

	r_count[i]++;
    if(tmp_min<min_local)
      min_local=tmp_min;
    for(i=0;i<nb_voisins_in;i++)
      if(r_count[i]<DIAMETRE)
        condD=1;
  } while(condD);
  printf("FIN Processus %d has min local %d.\n", rang, min_local);
}

/******************************************************************************/

int main (int argc, char* argv[]) {
   int nb_proc,rang;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

   if (nb_proc != NB_SITE+1) {
      printf("Nombre de processus incorrect !\n");
      MPI_Finalize();
      exit(2);
   }
  
   MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
   if (rang == 0) {
      simulateur();
   } else {
      calcul_min(rang);
   }
  
   MPI_Finalize();
   return 0;
}
