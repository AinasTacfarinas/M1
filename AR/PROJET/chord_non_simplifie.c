#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TAGINIT 0
#define LOOKUP 1
#define LASTCHANCE 2
#define ANSWER 3
#define FIN 4

#define NB_PAIR (nb_proc-1)


#define M 6

int nb_proc;
int rang;

int IDchord;
int succID[M];
int succMPI[M];
int resp;

int nb_IDchord;

void reception(int* valeur, int TAG);

void create_finger_table(int ID, int* ressuccID, int* ressuccMPI, int* tabid, int* tabtrie){
	int i,j,k,m,val,add;
	k=0;
	add=1;	
	for(i=0;i<M;i++){
		val = (ID + add)%nb_IDchord;
		for(j=0; j<NB_PAIR;j++){
			if(tabtrie[j]>=val){
				ressuccID[i] = tabtrie[j%NB_PAIR];
				for(m=0;m<NB_PAIR;m++){
					if(ressuccID[i] == tabid[m%NB_PAIR]){
						ressuccMPI[i] = m+1;
						break;
					}
				}
				break;
			}
		}
		if(j==NB_PAIR){
			ressuccID[i] = tabtrie[0];
			for(m=0;m<NB_PAIR;m++){
					if(ressuccID[i] == tabid[m%NB_PAIR]){
						ressuccMPI[i] = m+1;
						break;
					}
				}
			//k=0;
		}//else{
			k=j;
		//}
		add*=2;
	}
	
}

// fonction de tri d'un tableau en ordre croissant
void tri_function(int *tab, int size){
	int i, j;
	for (i = 1; i < size; ++i) {
       int elem = tab[i];
       for (j = i; j > 0 && tab[j-1] > elem; j--)
           tab[j] = tab[j-1];
       tab[j] = elem;
   }
}

// fonction de qui permet de savoir si on est reponsable d'une clé : retourne 1 si oui, sinon 0
int havekey(int key){
	if(resp<=IDchord)
		return resp <= key && key <=IDchord;
	else
		return ((resp <=key && key < nb_IDchord) || (0<=key && key <=IDchord));
}

int fingeri(int key,int finger){
	if(finger<=IDchord)
		return finger <= key && key <=IDchord;
	else
		return ((finger <=key && key < nb_IDchord) || (0<=key && key <=IDchord));
}

void rec_lookup(int* valeur){
	MPI_Status status;
	int i;
	int valeurec[5];
	if(havekey(valeur[2])){
		printf("%d rec lookup jai al cle\n",rang);
		fflush(stdout);
		valeur[3] = rang;
		valeur[4] = IDchord;
		MPI_Send(valeur, 5, MPI_INT, valeur[0], ANSWER, MPI_COMM_WORLD);
	}else{
		
		for(i=M-1;i>=0;i--){
			if(fingeri(valeur[2],succID[i])){
				MPI_Send(valeur, 5, MPI_INT, succMPI[i], LOOKUP, MPI_COMM_WORLD);
				break;
			}
		}
		if(i<0)
			MPI_Send(valeur,5 , MPI_INT, succMPI[0], LASTCHANCE, MPI_COMM_WORLD);
	}
	
	MPI_Recv(valeurec, 5, MPI_INT,MPI_ANY_SOURCE, FIN, MPI_COMM_WORLD, &status);
	
	reception(valeurec, status.MPI_TAG);
}

void rec_lastchance(int* valeur){
	MPI_Status status;
	int valeurec[5];
	if(havekey(valeur[2])){
		valeur[3] = rang;
		valeur[4] = IDchord;
		MPI_Send(valeur, 5, MPI_INT, valeur[0], ANSWER, MPI_COMM_WORLD);
		MPI_Recv(valeurec, 5, MPI_INT,MPI_ANY_SOURCE, FIN, MPI_COMM_WORLD, &status);
		reception(valeurec, status.MPI_TAG);
	}

}

void rec_answer(int* valeur){
	printf("C'est %d de rang %d qui a la cle %d\n",valeur[4],valeur[3],valeur[2]);
	fflush(stdout);
	MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
}

void rec_fin(int *valeur){
	if(valeur[0]!=succMPI[0])
		MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
}

void reception(int* valeur, int TAG){
	switch(TAG){
		case LOOKUP :
			rec_lookup(valeur);
			break;
		
		case LASTCHANCE :
			rec_lastchance(valeur);
			break;
		
		case ANSWER :
			rec_answer(valeur);
			break;
		
		case FIN :
			rec_fin(valeur);
			break;
		
		default :
			printf("TAG INCONNU!! %d\n",TAG);
			fflush(stdout);
			break;
	}
}

// un pair cherche une cle, les parametres ici ne servent uniquement à initialiser une recherche
void recherche(int chercheur, int cle_cherche){
	if(cle_cherche<0 || cle_cherche >= nb_IDchord){
		if(chercheur == rang)	
			printf("Impossible de rechercher la cle %d\n",rang, cle_cherche);
		return;
	}
	
	MPI_Status status;
	int i;
	int value[5] = {chercheur,-1,-1,-1,-1};
	int valeurec[5];
	if(chercheur == rang){
		printf("Je suis de rang %d et je recherche la cle %d\n", chercheur, cle_cherche);
		fflush(stdout);
		if(havekey(cle_cherche)){
			MPI_Send(value, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
			printf("JAI DEJA LA CLE!\n");
			fflush(stdout);
		}else{
			value[0]=chercheur;
			value[1]=IDchord;
			value[2]=cle_cherche;
			for(i=M-1;i>=0;i--){
				if(fingeri(cle_cherche,succID[i])){
					MPI_Send(value, 5, MPI_INT, succMPI[i], LOOKUP, MPI_COMM_WORLD);
					break;
				}
			}
			if(i<0){
				MPI_Send(value,5 , MPI_INT, succMPI[0], LASTCHANCE, MPI_COMM_WORLD);
				printf("j'ai envoyé %d %d %d\n",value[0],value[1],value[2]);
				fflush(stdout);
			}
			MPI_Recv(valeurec, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			printf("%d : JAI RECU %d %d %d %d %d\n",rang,valeurec[0],valeurec[1],valeurec[2],valeurec[3],valeurec[4]);
			fflush(stdout);
	
			reception(valeurec, status.MPI_TAG);
		}
	}else{
		MPI_Recv(valeurec, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		printf("%d : JAI RECU %d %d %d %d %d\n",rang,valeurec[0],valeurec[1],valeurec[2],valeurec[3],valeurec[4]);
		fflush(stdout);
	
		reception(valeurec, status.MPI_TAG);
	}
}


	


void simulateur(){

	int tab_IDchord[NB_PAIR]; // tab_IDchord[i] est l'ID chord du processus de rang MPI i+1, avec 0<=i<NB_PAIR
	int tab_IDchord_trie[NB_PAIR]; // tableau d'ID chord trié
	
	srand(time(NULL));
	
	int i,j,k;
	
	for(i=0;i<NB_PAIR;i++){
		tab_IDchord[i]=-1;
	}
	
	int alea;
	for(i=0;i<NB_PAIR;i++){
		alea = rand()%nb_IDchord;
		for(j=0;j<i;j++){
			if(alea==tab_IDchord[j]){
				i--;
				break;
			}
		}
		if(j==i){
			tab_IDchord[i]=alea;
			tab_IDchord_trie[i]=alea;
		}
	}
	
	for(i=0;i<NB_PAIR;i++){
		printf("%d\n",tab_IDchord[i]);
		fflush(stdout);
	}
	
	printf("\n");
	fflush(stdout);
	tri_function(tab_IDchord_trie,NB_PAIR);
   
	for(i=0;i<NB_PAIR;i++){
		printf("%d\n",tab_IDchord_trie[i]);
		fflush(stdout);

	}


	int nb,nb2,nb3;
	// Envoi de l'IDChord, de la première clé reponsable et de l'IDChord du successeur
	for(i=0; i<NB_PAIR; i++){
		MPI_Send(&tab_IDchord[i], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		for(j=0;j<NB_PAIR;j++){
			if(tab_IDchord[i]==tab_IDchord_trie[j]){
				if(j==0){
					nb=(tab_IDchord_trie[(NB_PAIR-1)]+1)%nb_IDchord;
				}else{
					nb=(tab_IDchord_trie[(j-1)]+1)%nb_IDchord;
				}
				
				MPI_Send(&nb, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
				
				create_finger_table(tab_IDchord[i],succID,succMPI, tab_IDchord, tab_IDchord_trie);
				MPI_Send(succID, M, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
				MPI_Send(succMPI, M, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
				
				break;
			}
		}
		
		
	}
	
}

void init(int rang){
	int i;
	MPI_Status status;
	
	MPI_Recv(&IDchord, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&resp, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(succID, M, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(succMPI, M, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	usleep(rang*10000);
	printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mes succID sont\n", rang,IDchord,resp);
	fflush(stdout);
	for(i=0;i<M;i++){
		printf("%d %d %d\n",i,succID[i],succMPI[i]);
		fflush(stdout);
	}
	
}





int main (int argc, char* argv[]) {
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
	
	nb_IDchord = (int)pow(2,M);

	if (nb_proc != NB_PAIR+1) {
		printf("Nombre de processus incorrect !\n");
		fflush(stdout);
		MPI_Finalize();
		exit(2);
	}
	
	if(rang == 0){
		simulateur();
		
	}else{
		init(rang);
	}
	
	
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		recherche(4,IDchord);
		
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		recherche(2,35);
	
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		recherche(5,100);
		
	
	
	MPI_Finalize();
	return 0;
}
