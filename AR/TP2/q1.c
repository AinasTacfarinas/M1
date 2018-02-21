#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define NB_MEALS 5

#define VEUX_BAGUETTE 0
#define TIENS_BAGUETTE 1
#define FINIS_MANGER 2


int main(int argc, char **argv){
	int my_rank;
	int nb_proc;
	int source;
	int dest;
	int provided;

	MPI_Status status;
  	int h=0;
	int msg;
	int bag_d = 0;
	int bag_g = 0;
	int tag;
  	int state = 0;
	int vg = 0;
	int vd = 0;
	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	int g,d;
	if((my_rank-1)<0)
		g = my_rank-1+nb_proc;
	else
		g = my_rank-1;
	
	d = (my_rank+1)%nb_proc;
	
	int i;
	for(i=0;i<NB_MEALS;i++){
		h++;
		MPI_Send(&h, 1, MPI_INT, g, VEUX_BAGUETTE, MPI_COMM_WORLD);
		MPI_Send(&h, 1, MPI_INT, d, VEUX_BAGUETTE, MPI_COMM_WORLD);
		while(state ==0){
			MPI_Recv(&msg, 1, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(status.MPI_TAG==VEUX_BAGUETTE)
				if(msg<h || (msg==h && status.MPI_SOURCE<my_rank)){
					h = msg + 1;
					h++;
					MPI_Send(&h, 1, MPI_INT, status.MPI_SOURCE, TIENS_BAGUETTE, MPI_COMM_WORLD);
					if(status.MPI_SOURCE == g)
						bag_g = 0;
					else
						bag_d = 0;
				}else{
					h++;
					if(status.MPI_SOURCE == g)
						vg = 1;
					else
						vd = 1;
				}
			else
				if(status.MPI_SOURCE == g)
					bag_g = 1;
				else
					bag_d = 1;
			
			if(bag_d == 1 && bag_g == 1)
				state=1;
		}
		sleep(5);
		printf("je suis %d et j'ai boufé\n",my_rank);
		bag_g = 0;
		bag_d = 0;
		state=0;
		if(vg==1){
			h++;
			MPI_Send(&h, 1, MPI_INT, g, FINIS_MANGER, MPI_COMM_WORLD);
			vg=0;
		}
		if(vd==1){
			h++;
			MPI_Send(&h, 1, MPI_INT, d, FINIS_MANGER, MPI_COMM_WORLD);
			vd=0;
		}
	
			
	}
	
	
	MPI_Finalize();

	return 0;

}
