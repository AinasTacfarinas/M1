#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LOOKUP 1
#define LASTCHANCE 2
#define ANSWER 3
#define FIN 4
#define CONNEXION 5
#define REPONSE_CONNEXION 6
#define NB_PAIR nb_proc
#define M 6

int nb_proc;
int rang;

int IDchord;
int succID[M];
int succMPI[M];
int resp;

int nb_IDchord;

void reception(int* valeur, int TAG);

int fingeri2(int id, int key,int finger){
	if(id<finger)
		return id < key && key <=finger;
	else
		return ((id <key && key < nb_IDchord) || (0<=key && key <=finger));
}

void maj_finger_table(int* ressuccID, int* ressuccMPI, int newID, int newrang, int oldID){
	
	int i;
	
	for(i=0;i<M;i++){
			int tmp = (IDchord+ (int)pow(2,i))%nb_IDchord;
			if(fingeri2(IDchord,tmp,newID)){
				ressuccID[i] = newID;
				ressuccMPI[i] = newrang;
			}
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

//Est-ce que key appartient à [finger;IDchord]
int fingeri(int key,int finger){
	if(finger<=IDchord)
		return finger <= key && key <=IDchord;
	else
		return ((finger <=key && key < nb_IDchord) || (0<=key && key <=IDchord));
}

void rec_lookup(int* valeur){
	MPI_Status status;
	int i;
	if(havekey(valeur[2])){
		printf("%d rec lookup jai al cle\n",rang);
		fflush(stdout);
		valeur[3] = rang;
		valeur[4] = IDchord;
		MPI_Send(valeur, 5, MPI_INT, valeur[0], ANSWER, MPI_COMM_WORLD);
		if(valeur[0]!=succMPI[0])
			MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
	}else{
		for(i=M-1;i>=0;i--){
			if(fingeri(valeur[2],succID[i])){
				MPI_Send(valeur, 5, MPI_INT, succMPI[i], LOOKUP, MPI_COMM_WORLD);
				if(succMPI[i]!=succMPI[0]){
					valeur[0] = succMPI[i];
					MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
				}
				break;
			}
		}
		if(i<0)
			MPI_Send(valeur,5 , MPI_INT, succMPI[0], LASTCHANCE, MPI_COMM_WORLD);
	}
}

void rec_lastchance(int* valeur){
	MPI_Status status;
	int valeurec[5];
	if(havekey(valeur[2])){
		valeur[3] = rang;
		valeur[4] = IDchord;
		MPI_Send(valeur, 5, MPI_INT, valeur[0], ANSWER, MPI_COMM_WORLD);
		if(succMPI[0]!=valeur[0])
			MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
	}
}

void rec_answer(int* valeur){
	printf("C'est %d de rang %d qui a la cle %d\n",valeur[4],valeur[3],valeur[2]);
	fflush(stdout);
}

void rec_fin(int *valeur){
	if(valeur[0]!=succMPI[0])
		MPI_Send(valeur, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
}

void rec_connexion(int* valeur){
	if(resp==IDchord){
		MPI_Status status;
		int value[4+2*M];
		MPI_Send(valeur, 4+2*M, MPI_INT, succMPI[0], CONNEXION, MPI_COMM_WORLD);
		printf("JE SEND\n");
		fflush(stdout);
		MPI_Recv(value, 4+2*M, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		reception(value,status.MPI_TAG);
	}else{
		int nbkey,newIDchord;
		if(resp<IDchord)
			nbkey=IDchord-resp;
		else
			nbkey=nb_IDchord-resp + IDchord;
			
		newIDchord=(resp+rand()%nbkey)%nb_IDchord;
		printf("NEW ID CHORD %d\n",newIDchord);
		fflush(stdout);
		
		valeur[1] = newIDchord;
		valeur[2] = resp;
		int indice = 0;
		valeur[4+indice] = IDchord;
		valeur[4+M+indice] = rang;
		valeur[3] = indice;
		
		int i;
		for(i=0;i<M;i++){
			int tmp = (newIDchord+(int)pow(2,i))%nb_IDchord;
			printf("entrant %d key %d pair %d\n",valeur[1],tmp,IDchord);
			fflush(stdout);
			if(fingeri2(newIDchord,tmp,IDchord)){
				valeur[4+i]=IDchord;
				valeur[4+M+i]=rang;
			}else
				break;
		}
		valeur[3] = i;
		for(i=valeur[3];i<M;i++){
				valeur[4+i]=newIDchord;
				valeur[4+M+i]=valeur[0];
		}
		resp = (newIDchord+1)%nb_IDchord;
		printf("JE send %d %d %d %d %d\n",valeur[0],valeur[1],valeur[2],valeur[3],valeur[4]);
		fflush(stdout);
		int anciensucc = succMPI[0];
		maj_finger_table(succID, succMPI,newIDchord , valeur[0], IDchord);
		if(anciensucc != rang){
			MPI_Send(valeur, 4+2*M, MPI_INT, anciensucc, REPONSE_CONNEXION, MPI_COMM_WORLD);
			printf("JE SEND REPONSE_CONNEXION\n");
			fflush(stdout);
		}
		
	}
}

void rec_reponseconnexion(int* valeur){
	int i;
	printf("ALLO indice : %d\n",valeur[3]);
	fflush(stdout);
	
	if(rang == valeur[0]){
		IDchord = valeur[1];
		for(i=valeur[3];i<M;i++){
		int tmp = (valeur[1]+(int)pow(2,i))%nb_IDchord;
		printf("%d ALLLO entrant %d key %d pair %d\n",rang,valeur[1],tmp,IDchord);
		fflush(stdout);
		if(fingeri2(valeur[1],tmp,IDchord)){
			valeur[4+i]=IDchord;
			valeur[4+M+i]=rang;
			valeur[3]++;
		}else
			break;
	}
		for(i=0;i<M;i++){
			succID[i] = valeur[4+i];
			succMPI[i] = valeur[4+M+i];
		}
		resp = valeur[2];
	}else{
		for(i=valeur[3];i<M;i++){
		int tmp = (valeur[1]+(int)pow(2,i))%nb_IDchord;
		printf("%d ALLO entrant %d key %d pair %d\n",rang,valeur[1],tmp,IDchord);
		fflush(stdout);
		if(fingeri2(valeur[1],tmp,IDchord)){
			valeur[4+i]=IDchord;
			valeur[4+M+i]=rang;
			valeur[3]++;
		}else
			break;
	}
		printf("ancien succ %d\n",succMPI[0]);
		fflush(stdout);
		maj_finger_table(succID, succMPI,valeur[1] , valeur[0], IDchord);
		for(i=0;i<M;i++){
			printf("%d : %d\n",i,succMPI[i]);
			fflush(stdout);
		}
		printf("%d JE SEND REPONSE_CONNEXION a %d\n",succMPI[0]);
		fflush(stdout);
		MPI_Send(valeur, 4+2*M, MPI_INT, succMPI[0], REPONSE_CONNEXION, MPI_COMM_WORLD);
		printf("%d JE SEND REPONSE_CONNEXION a %d\n",succMPI[0]);
		fflush(stdout);
	}
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
		
		case CONNEXION :
			rec_connexion(valeur);
			break;
		
		case REPONSE_CONNEXION :
			rec_reponseconnexion(valeur);
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
			printf("Impossible de rechercher la cle %d\n", cle_cherche);
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
			return;
		}else{
			value[0]=chercheur;
			value[1]=IDchord;
			value[2]=cle_cherche;
			for(i=M-1;i>=0;i--){
				if(fingeri(cle_cherche,succID[i])){
					MPI_Send(value, 5, MPI_INT, succMPI[i], LOOKUP, MPI_COMM_WORLD);
					if(succMPI[i]!=succMPI[0]){
						value[0]=succMPI[i];
						MPI_Send(value, 5, MPI_INT, succMPI[0], FIN, MPI_COMM_WORLD);
					}
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


void connexion(int initiateur, int rangpair){
	printf("CONNEXION\n");
	fflush(stdout);
	MPI_Status status;
	int value[4+2*M];
	int tabconnexion[4+2*M] = {rang};
	if(initiateur==rang){
		printf("%d Veut se connecter sur %d\n", initiateur,rangpair);
		fflush(stdout);
		MPI_Send(tabconnexion, 4+2*M, MPI_INT, rangpair, CONNEXION, MPI_COMM_WORLD);
		printf("JE SEND\n");
		fflush(stdout);
	}
	MPI_Recv(value, 4+2*M, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	printf("%d recoit %d de %d\n",rang,status.MPI_TAG,status.MPI_SOURCE);
	reception(value,status.MPI_TAG);
}



void init(int initiateur){
	
	if(rang == initiateur){
		srand(time(NULL));
		IDchord = rand()%nb_IDchord;
		resp = (IDchord+1)%nb_IDchord;
		int i;
		for(i=0;i<M;i++){
			succID[i]=IDchord;
			succMPI[i]=rang;
		}
	}
}





int main (int argc, char* argv[]) {
	int i;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
	
	nb_IDchord = (int)pow(2,M);

	if (nb_proc > nb_IDchord) {
		printf("Nombre de processus incorrect !\n");
		fflush(stdout);
		MPI_Finalize();
		exit(2);
	}
	
	
	
	if(rang == 0){
		init(rang);
		printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mes succID sont\n", rang,IDchord,resp);
	fflush(stdout);
		for(i=0;i<M;i++){
			printf("%d %d %d\n",i,succID[i],succMPI[i]);
			fflush(stdout);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD );
	
	if(rang==0 || rang ==1)
		connexion(1,0);
	
	
	MPI_Barrier(MPI_COMM_WORLD );
	if(rang==0 || rang ==1){
		usleep(rang*10000);
		printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mes succID sont\n", rang,IDchord,resp);
		fflush(stdout);
		for(i=0;i<M;i++){
			printf("%d %d %d\n",i,succID[i],succMPI[i]);
			fflush(stdout);
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD );
	connexion(2,0);
	MPI_Barrier(MPI_COMM_WORLD );
	
	usleep(rang*10000);
	printf("Je suis de rang %d et j'ai pour ID %d et mon resp est %d et mes succID sont\n", rang,IDchord,resp);
	fflush(stdout);
	for(i=0;i<M;i++){
		printf("%d %d %d\n",i,succID[i],succMPI[i]);
		fflush(stdout);
	}
	
	/*
	
	
	for(i=1;i<nb_proc;i++){
		
		if(rang==i){
			srand(time(NULL));
			int rangpair=rand()%i;
			connexion(rang,rangpair);
		MPI_Barrier(MPI_COMM_WORLD );
	}
	*/	
	
	
	MPI_Finalize();
	return 0;
}
