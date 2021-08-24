#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>

unsigned int *A, *C;
int A_id, C_id;

unsigned int *connectSharedMemory(int dimension, int *id);    //store matrix in shared memory
void workInChild(int lowerbound, int upperbound, int dimension);
void detachSharedMemory();
void detachAndReleaseSharedMemory();

int main()
{
    int dimension;
    printf("Input the matrix dimension:");
    scanf("%d", &dimension);    //read in matrix dimension

    A = connectSharedMemory(dimension, &A_id);
   
    for(unsigned int i=0; i<dimension; i++)  //initialize matrix
    {
        for(unsigned int j=0; j<dimension; j++)
        {
            A[i*dimension + j] = i*dimension + j;
        }
    }


    for(int i=1; i<=16; i++)
    {
        int sec, usec, lowerbound = 0, upperbound = dimension/i;
        unsigned int sum = 0;
        struct timeval start, end;

        C = connectSharedMemory(dimension, &C_id);
        
        pid_t pid[i];
        gettimeofday(&start, 0);

        for(int j=0; j<i; j++)
        {
            pid[j] = fork();
            if (pid[j] < 0) //error occurred
            {
                fprintf(stderr, "Fork Failed");
                exit(-1);
            }
            else if (pid[j] == 0) //child process->execute command
            {
                workInChild(lowerbound, upperbound, dimension);
                //printf("%d %d\n", lowerbound, upperbound);
                exit(0);
            }

            lowerbound = upperbound;
            upperbound += dimension/i;
            if(upperbound > dimension || j == i-2) upperbound = dimension;
        }

        int status, child = i;
        pid_t pids;
        while (child > 0) //wait for all child to finish
        {
            pids = wait(&status);
            --child;
        }

        sum = 0;
        for(unsigned int i=0; i < (dimension * dimension); i++)  //sum
        {
            sum += C[i];
            // printf("%u ", C[i]);
        }
        printf("\n");

        detachAndReleaseSharedMemory();

        gettimeofday(&end, 0);
        sec = end.tv_sec - start.tv_sec;
        usec = end.tv_usec - start.tv_usec;
        printf("Multiplying matrices using %d process\n", i);
        printf("Elapsed time: %f sec, ", sec + (usec/1000000.0));
        printf("Checksum: %u\n", sum);
        
    }


}

unsigned int *connectSharedMemory(int dimension, int *id)
{
    *id = shmget(IPC_PRIVATE, dimension * dimension *sizeof(unsigned int), IPC_CREAT | 0666);
    if (*id < 0) 
    {
        printf("shmget error\n");
        exit(1);
    }

    unsigned int *matrix;
    matrix = shmat(*id, NULL, 0);
    if(matrix < (unsigned int *)0)
    {
        printf("shmat error\n");
        exit(1);
    }

    return matrix;
}

void workInChild(int lowerbound, int upperbound, int dimension)
{
    for(int i=lowerbound; i<upperbound; i++)
    {
        for(int j=0; j<dimension; j++)
        {
            C[i*dimension +j] = 0;
            for(int k=0; k<dimension; k++)
            {
                C[i*dimension + j] += A[i*dimension + k] * A[j + k*dimension];
            } 
        }
    }

    detachSharedMemory();
}

void detachAndReleaseSharedMemory()
{
    detachSharedMemory();
    if (shmctl(C_id, IPC_RMID, 0) == -1) 
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(1);
    }
}

void detachSharedMemory()
{
    if (shmdt(C) == -1) 
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
}
