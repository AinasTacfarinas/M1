#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TAGINIT 0
#define LOOKUP 1
#define REPONSE_LOOKUP 2
#define DECONNEXION 3
#define REPONSE_DECONNEXION 4
#define RECONNEXION 5
#define REPONSE_RECONNEXION 6
#define IGNORER 7


#define NB_PAIR (nb_proc-1)


#define M 6

int nb_proc;
int rang;

int IDchord;
int succID;
int succMPI;
int resp;

int nb_IDchord;

void reception(int* valeur, int TAG);

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


void rec_lookup(int* valeur){
	
		if(havekey(valeur[1])){
			valeur[2]=IDchord;
			MPI_Send(valeur, 3, MPI_INT, succMPI, REPONSE_LOOKUP, MPI_COMM_WORLD);
		}else{
			MPI_Send(valeur, 3, MPI_INT, succMPI, LOOKUP, MPI_COMM_WORLD);
		}
	
}

void rec_reponselookup(int* valeur){
	if(IDchord == valeur[0]){
		printf("Je suis le demandeur de rang %d et d'ID %d et j'ai recherché la cle %d. C'est %d qui l'a !\n",rang, IDchord,valeur[1],valeur[2]);
		fflush(stdout);
	}else{
		MPI_Send(valeur, 3, MPI_INT, succMPI, REPONSE_LOOKUP, MPI_COMM_WORLD);
	}
}

void rec_deconnexion(int* valeur){
	resp = valeur[0];
	valeur[2]=IDchord;
	valeur[3]=rang;
	MPI_Send(valeur, 4, MPI_INT, succMPI, REPONSE_DECONNEXION, MPI_COMM_WORLD);
}

void rec_reponsedeconnexion(int* valeur){
	if(succID==valeur[1]){
		MPI_Send(valeur, 4, MPI_INT, succMPI, REPONSE_DECONNEXION, MPI_COMM_WORLD);
		succID = valeur[2];
		succMPI = valeur[3];
	}else{
		if(IDchord ==valeur[1]){
			printf("%d JE PEUX DECO\n",rang);
			fflush(stdout);
		}else{
			MPI_Send(valeur, 4, MPI_INT, succMPI, REPONSE_DECONNEXION, MPI_COMM_WORLD);
		}
	}
}

void rec_reconnexion(int* valeur){
	if(resp==IDchord){
		MPI_Status status;
		int value[5];
		MPI_Send(valeur, 5, MPI_INT, succMPI, RECONNEXION, MPI_COMM_WORLD);
		MPI_Recv(value, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		reception(value,status.MPI_TAG);
	}else{
		int nbkey,newIDchord;
		if(resp<IDchord)
			nbkey=IDchord-resp;
		else
			nbkey=nb_IDchord-resp + IDchord;
		newIDchord=resp+rand()%nbkey;
		
		
		valeur[1] = newIDchord;
		valeur[2] = IDchord;
		valeur[3] = rang;
		valeur[4] = resp;
		resp = newIDchord+1;
		printf("JE send %d %d %d %d\n",valeur[0],valeur[1],valeur[2],valeur[3]);
		MPI_Send(valeur, 5, MPI_INT, succMPI, REPONSE_RECONNEXION, MPI_COMM_WORLD);
	}
}


void rec_reponsereconnexion(int* valeur){
	if(rang == valeur[0]){
		IDchord = valeur[1];
		succID = valeur[2];
		succMPI = valeur[3];
		resp = valeur[4];
	}else{
		if(succID ==valeur[2]){
			succID = valeur[1];
			succMPI = valeur[0];
		}
		MPI_Send(valeur, 5, MPI_INT, succMPI, REPONSE_RECONNEXION, MPI_COMM_WORLD);
	}
}

void rec_ignorer(int* valeur){
	printf("%d j'ignore le message\n",rang);
	if(IDchord == valeur[0]){
		if(havekey(valeur[1])){
			printf("J'ai reçu lookup alors que j'avais déjà la clé, le message a tourné uniquement pour débloquer les Recv des autres processus\n");
			fflush(stdout);
		}else{
			printf("PERSONNE N'A LA CLE %d! \n",valeur[1]);
			fflush(stdout);
		}
		
	}else{
		MPI_Send(valeur, 3, MPI_INT, succMPI, IGNORER, MPI_COMM_WORLD);
	}
}

void reception(int* valeur, int TAG){
	switch(TAG){
		case LOOKUP :
			rec_lookup(valeur);
			break;
		
		case REPONSE_LOOKUP :
			rec_reponselookup(valeur);
			break;
			
		case DECONNEXION :
			 rec_deconnexion(valeur);
			break;
			
		case REPONSE_DECONNEXION :
			 rec_reponsedeconnexion(valeur);
			break;
			
		case RECONNEXION :
			 rec_reconnexion(valeur);
			break;
			
		case REPONSE_RECONNEXION :
			 rec_reponsereconnexion(valeur);
			break;
			
		case IGNORER :
			 rec_ignorer(valeur);
			break;
		
		default :
			printf("TAG INCONNU!!\n");
			fflush(stdout);
			break;
	}
}

// un pair cherche une cle, les parametres ici ne servent uniquement à initialiser une recherche
void recherche(int chercheur, int cle_cherche){
	MPI_Status status;
	int value[4];
	int tabrecherche[3] = {IDchord,cle_cherche};	
	if(chercheur == rang){
		printf("Je suis le demandeur de rang %d et d'ID %d et je recherche la cle %d\n",rang, IDchord,tabrecherche[1]);
		if(havekey(cle_cherche)){
			printf("JAI DEJA LA CLE mais j'envoie tout de meme un message de type ignorer pour debloquer les autres processsus\n");
			fflush(stdout);
			MPI_Send(tabrecherche, 3, MPI_INT, succMPI, IGNORER, MPI_COMM_WORLD);
		}else{
			MPI_Send(tabrecherche, 3, MPI_INT, succMPI, LOOKUP, MPI_COMM_WORLD);
		}
	}
	MPI_Recv(value, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	reception(value,status.MPI_TAG);
}

// Un pair se déconnecte, le signal à son successeur, qui devient le successeur et de son prédecesseur, en lui envoyant la première clé dont il est responsable
void deconnexion(int initiateur){
	MPI_Status status;
	int value[4];
	int tabdeconnexion[4] = {resp,IDchord};
	if(initiateur == rang){
		MPI_Send(tabdeconnexion, 4, MPI_INT, succMPI, DECONNEXION, MPI_COMM_WORLD);
	}
	MPI_Recv(value, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	reception(value,status.MPI_TAG);

}

// Un pair qui s'est déconnecté tente de se reconnecter, il envoie poru cela une demande de reconnexion à un pair du système
void reconnexion(int initiateur, int pair){
	MPI_Status status;
	int value[5];
	int tabreconnexion[5] = {rang};
	if(initiateur==rang){
		printf("%d Veut se reconnecter sur %d\n", initiateur,pair);
		MPI_Send(tabreconnexion, 5, MPI_INT, pair, RECONNEXION, MPI_COMM_WORLD);
	}
	MPI_Recv(value, 5, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	reception(value,status.MPI_TAG);
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
				
				nb2=tab_IDchord_trie[(j+1)%(NB_PAIR)];
				
				MPI_Send(&nb, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
				MPI_Send(&nb2, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
				
				break;
			}
		}
		
		
	}
	
	// Envoi de l'id MPI du successeur
	for(i=0; i<NB_PAIR; i++){
		for(j=0;j<NB_PAIR;j++){
		
			if(tab_IDchord_trie[i]==tab_IDchord[j]){
				k=j+1;
			}
		
			if(tab_IDchord_trie[(i+1)%(NB_PAIR)]==tab_IDchord[j]){
				nb3=j+1;
			}
			
		}
		MPI_Send(&nb3, 1, MPI_INT, k, TAGINIT, MPI_COMM_WORLD);
	}
	
	
}

void init(int rang){
	
	MPI_Status status;
	
	MPI_Recv(&IDchord, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&resp, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&succID, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&succMPI, 1, MPI_INT,0, TAGINIT, MPI_COMM_WORLD, &status);
	printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mon succID est %d et mon succMPI est %d\n", rang,IDchord,resp,succID,succMPI);
	fflush(stdout);
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
		recherche(1,IDchord);
		
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		recherche(2,50);
		
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		deconnexion(1);
		
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mon succID est %d et mon succMPI est %d\n", rang,IDchord,resp,succID,succMPI);
	fflush(stdout);
	
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		reconnexion(1,3);
	
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang != 0)
		printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mon succID est %d et mon succMPI est %d\n", rang,IDchord,resp,succID,succMPI);
	fflush(stdout);
	
	MPI_Finalize();
	return 0;
}
