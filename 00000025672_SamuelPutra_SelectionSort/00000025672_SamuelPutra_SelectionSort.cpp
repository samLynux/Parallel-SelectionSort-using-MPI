#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include <windows.h>
#include <malloc.h>

int smallest(int*, int, int);

int main(int argc, char** argv) 
{
	
	MPI_Init(&argc, &argv);
	
	int world_rank;
	int world_size;


	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	int n = world_size * world_size;
	int* arrayA = (int*)malloc(n * sizeof(int));
	int c;
	srand(time(NULL));
	if (world_rank == 0) {
		printf("This is the unsorted array: ");
		for (c = 0; c < n; c++) {

			if (c % world_size == 0)
				printf("\n");
			arrayA[c] = rand() % 100;
			printf("%3d ", arrayA[c]);

		}

		printf("\n");
		printf("\n");
	}
	
	int size = n / world_size;
	
	
	int* selected = NULL;
	int smallestValue = NULL;
	int smallestInProcess;
	if (world_rank == 0) {

		selected = (int*)malloc(n * sizeof(int));

	}
	int* arrayB = (int*)malloc(n * sizeof(int));


	MPI_Scatter(arrayA, size, MPI_INT, arrayB, size, MPI_INT, 0, MPI_COMM_WORLD);
	smallestInProcess = smallest(arrayB, 0, size);

	
	
	
	
	int processNumber = 0;
	int startingPoint = 0;
	boolean isEmpty = false;

	MPI_Barrier(MPI_COMM_WORLD);
	
	for (int a = 0; a < n; a++)
	{
		processNumber = 0;
		if (world_rank == 0)
		{
			if(!isEmpty)
				smallestValue = smallestInProcess;
			else
			{
				smallestValue = 100;
			}
		}
			
		for (int b = 1; b < world_size; b++)
		{
			if (world_rank == 0) {
				int receive;
				MPI_Recv(&receive, 1, MPI_INT, b, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				if(receive != 100)
					if (receive < smallestValue)
					{
						smallestValue = receive;
						processNumber = b;
					}
			}
			else if (world_rank == b)
			{
				if(!isEmpty)
					MPI_Send(&smallestInProcess, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				else
				{
					int x = 100;
					MPI_Send(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				}
			}

		}
		
		MPI_Bcast(&processNumber, 1, MPI_INT, 0, MPI_COMM_WORLD);

		MPI_Barrier(MPI_COMM_WORLD);
		if (world_rank == 0)
		{
			selected[a] = smallestValue;
		}
		if (world_rank == processNumber)
		{
			startingPoint++;
			smallestInProcess = smallest(arrayB, startingPoint, size);
			if (startingPoint > size-1)
				isEmpty = true;
			
		}

	}

	if (world_rank == 0) {	
		printf("\nThis is the sorted array: ");
		for (c = 0; c < n; c++) {
			if (c % world_size == 0)
				printf("\n");
			printf("%3d ", selected[c]);

		}

		printf("\n");
		printf("\n");
	}

	free(arrayA);
	free(arrayB);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}


int smallest(int* a,int b, int c)
{
	int temp = b;
	for (int x = b + 1; x < c; x++)
	{
		if (a[temp] > a[x])
			temp = x;
	}
	int z = a[temp];
	a[temp] = a[b];
	a[b] = z;
	
	return a[b];
}