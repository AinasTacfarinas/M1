#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define TAGINIT 0
#define NB_SITE 6

void simulateur(void) {
	int i;
	/* nb_voisins[i] est le nombre de voisins du site i */
	int nb_voisins[NB_SITE+1] = {-1, 2, 3, 2, 1, 1, 1};
	int min_local[NB_SITE+1] = {-1, 3, 11, 8, 14, 5, 17};
	/* liste des voisins */
	int voisins[NB_SITE+1][3] = {{-1, -1, -1},
	{2, 3, -1}, {1, 4, 5},
	{1, 6, -1}, {2, -1, -1},
	{2, -1, -1}, {3, -1, -1}};
	for(i=1; i<=NB_SITE; i++){
		MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(voisins[i],nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
	}
}


void calcul_min(int rang){
	
	MPI_Status status;
	int nb_voisins, min_local;
	MPI_Recv(&nb_voisins, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	int voisins[nb_voisins];
	MPI_Recv(voisins, nb_voisins, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&min_local, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	
	int recu[nb_voisins];
	int inverse[NB_SITE+1];	
	int i;
	for(i=0;i<nb_voisins;i++){
		recu[i]=0;
		inverse[voisins[i]]=i;
	}
	int sent = 0;
	int last = 0;
	int fin = 0;
	int q;
	int condS;
	int condD;
	int x;
	
	do{
		condS = 0;
		condD = 0;
		if(sent==0){
			for(i=0;i<nb_voisins;i++){
				if(recu[i]==0){
					q=voisins[i];
					condS++;
				}
			}
			if(condS==1){
				MPI_Send(&min_local, 1, MPI_INT, q, TAGINIT, MPI_COMM_WORLD);
				sent = 1;
			}
		}
		
		MPI_Recv(&x, 1, MPI_INT,MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
		if(x<min_local)
			min_local = x;
		recu[inverse[status.MPI_SOURCE]] = 1;
		last = status.MPI_SOURCE;
		
		for(i=0;i<nb_voisins;i++){
			if(recu[i]==1){
				condD++;
			}
		}
	}while(condD!=nb_voisins);
	printf("%d le min est %d\n",rang,min_local);
	for(i=0;i<nb_voisins;i++)
		if(last!=voisins[i])
			MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGINIT, MPI_COMM_WORLD);
}

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
