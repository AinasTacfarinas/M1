#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define TAGREQ 0
#define TAGREP 1
#define NB_SITE 6
#define MAX_CS 6


void op(int rang) {


	MPI_Status status;
	int nb_demande_sc = 0;
	int h = 0;
	int FA[NB_SITE-1];
	int nbfile;
	int date_req;
	int etat = 0; /* 0=not_req 1=req 2=SC*/
	int nb_reply;
	int i;
	int htmp;
	
	while(nb_demande_sc<MAX_CS){
	
		/*REQUEST*/
		h++;
		etat = 1;
		date_req = h;
		nb_reply = 0;
		nbfile=0;
		for(i=0;i<NB_SITE;i++)
			if(i!=rang)
				MPI_Send(&h, 1, MPI_INT, i, TAGREQ, MPI_COMM_WORLD);

		/**************/
		
		/*REC*/
		while(nb_reply != NB_SITE-1){
			MPI_Recv(&htmp, 1, MPI_INT, MPI_ANY_SOURCE, TAGREQ, MPI_COMM_WORLD, &status);
			if(rang==5)
				printf("Je recois %d,%d de %d\n", status.MPI_TAG,htmp,status.MPI_SOURCE);
			if(htmp > h)
				h = htmp;
			h++;

			if((etat==0) || ((etat==1)&&((htmp<date_req)||((htmp==date_req)&&(status.MPI_SOURCE<rang))))){
				MPI_Send(&h, 1, MPI_INT, status.MPI_SOURCE, TAGREP, MPI_COMM_WORLD);
			}else{
				FA[nbfile]=status.MPI_SOURCE;
				nbfile++;
			}
			nb_reply++;
		 }	
		 nb_reply = 0;
		 while(nb_reply != NB_SITE-1){
			MPI_Recv(&htmp, 1, MPI_INT, MPI_ANY_SOURCE, TAGREP, MPI_COMM_WORLD, &status);
			if(status.MPI_TAG== TAGREP){
				nb_reply++;
			}
		}
		etat = 2;
		printf("%d est en SC\n",rang);
		/*************************/
		
		/*RELEASE*/
		h++;
		etat=0;
		for(i=0;i<nbfile;i++){
			MPI_Send(&h, 1, MPI_INT, FA[i], TAGREP, MPI_COMM_WORLD);
		}
		nbfile=0;
		nb_demande_sc++;
		/****************************/
	}
	printf("FIN de %d\n",rang);

}

/******************************************************************************/

int main (int argc, char* argv[]) {

	int nb_proc,rang;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
	
	if (nb_proc != NB_SITE) {
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}
	
	op(rang);
	  
	MPI_Finalize();
	return 0;
}
