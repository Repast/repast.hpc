#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv){
	int rank;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if(rank == 0){
	    printf("Hello, world! I'm rank %d\n", rank);
	}
	else{
	    printf("Hmm...\n");
	}
	
	MPI_Finalize();
}