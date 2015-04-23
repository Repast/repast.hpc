#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv){
	int rank;
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	printf("Hello, world! I'm rank %d\n", rank);

	MPI_Finalize();
}
