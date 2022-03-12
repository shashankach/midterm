#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdbool.h>
#define MATRIX_DIMENSION_XY 10



void swap(int* arr,int i,int j){
    int n = arr[i];
    arr[i] = arr[j];
    arr[j] = n;
}

void printarray(int* list,int length){
    for (int i=0;i<length;i++){
        printf("%d ",list[i]);
    }
    printf("\n");
}


void oddSort(int* arr, int n,int par_id,int par_count)
{
    bool isSorted = false; // Initially array is unsorted
    int start = 32 *((float) par_id/(float)par_count);
    int end = 32*(((float) (par_id+1))/(float)par_count);
    //printf("%d/%d==>%d:%d|\n", par_id, par_count, start, end);
    //printf("startodd: %d\n",start);
    //printf("endodd: %d\n",end);
    //printf("ODD: %d/%d->%d:%d\n", par_id, par_count, start, end);

    //while (!isSorted) {
     //   isSorted = true;
  
        // Perform Bubble sort on odd indexed element
        for (int i = start+1 ; i < end-1; i = i + 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr,i, i + 1);
                //isSorted = false;
            }
        }
  
    //}    
  
    return;
}


void EvenSort(int* arr, int n, int par_id,int par_count)
{
    bool isSorted = false; // Initially array is unsorted
    int start = 32 *((float) par_id/(float)par_count);
    int end = 32*(((float) (par_id+1))/(float)par_count);
    //printf("starteven: %d\n",start);
    //printf("endeven: %d\n",end);
    //printf("EVEN: %d/%d->%d:%d\n", par_id, par_count, start, end);
   
    
        // Perform Bubble sort on even indexed element
        for (int i = start; i < end-1; i = i + 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr, i, i+1);
                //isSorted = false;
            }
        }
    
  
    return;
}


//************************************************************************************************************************
void synch(int par_count,int *ready,int par_id)
{
    int synchid = ready[par_count]+1;
    ready[par_id]=synchid;
    int breakout = 0;
    while(1)
    {
        breakout=1;
        for(int i=0;i<par_count;i++)
        {
            if(ready[i]<synchid)
            {
                breakout = 0;
                break;
            }
        }
        if(breakout==1)
        {
            ready[par_count] = synchid;
            break;
        }
    }
}


int main(int argc, char *argv[])
{

    struct timeval t1,t2;
    gettimeofday(&t1,NULL);
    int par_id = 0; // the parallel ID of this process
    int par_count = 1; // the amount of processes
    int *list; //matrices A,B and C
    int *ready; //needed for synch

    if(argc!=3){printf("no shared\n");}
    else
        {
        par_id= atoi(argv[1]);
        par_count= atoi(argv[2]);
    // strcpy(shared_mem_matrix,argv[3]);
        }
    if(par_count==1){
        printf("only one process\n");
    }

    int fd[2];
    if(par_id==0)
        {
            //TODO: init the shared memory for A,B,C, ready. shm_open with C_CREAT here! then ftruncate! then mmap
            fd[0] = shm_open("list", O_CREAT|O_RDWR, 0600);
            ftruncate(fd[0], sizeof(int)*32);
            list = (int*)mmap(NULL, sizeof(int)*32, PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);

            fd[1] = shm_open("ready", O_CREAT|O_RDWR, 0600);
            ftruncate(fd[1], sizeof(int)*par_count);
            ready = (int*)mmap(NULL, sizeof(int) *par_count, PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
            for (int i=0; i<11; i++){
                ready[i] = 0;
            }
        }
    else
        {
        //TODO: init the shared memory for A,B,C, ready. shm_open withOUT C_CREAT here! NO ftruncate! but yes to mmap

            sleep(2); //needed for initalizing synch
           
            fd[0] = shm_open("list", O_RDWR, 0600);
            list = (int*)mmap(NULL, sizeof(int)*32, PROT_READ|PROT_WRITE, MAP_SHARED, fd[0], 0);

            fd[1] = shm_open("ready", O_RDWR, 0600);
            ready = (int*)mmap(NULL, sizeof(int) * par_count, PROT_READ|PROT_WRITE, MAP_SHARED, fd[1], 0);
        }
    synch(par_count, ready, par_id);

    if(par_id==0)
        {
        //TODO: initialize the matrices A and B
        int bruh = 32;
        for(int i=0;i<32;i++){
            //list[i]  =bruh;
            //bruh-=1;
            list[i] = 1000 * ((float) rand())/((float) RAND_MAX);
        }
        printarray(list, 32);
    }
    
    synch(par_count, ready, par_id);

    //TODO: quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C, ...);

    for(int i=0;i<32;i++){
        oddSort(list,32,par_id,par_count);
        synch(par_count, ready, par_id);
        EvenSort(list,32,par_id,par_count);
        synch(par_count, ready, par_id);

    }
    
    
        
    

    if(par_id==0)
        //quadratic_matrix_print(C);
        printarray(list, 32);
    synch(par_count, ready, par_id);

    //lets test the result:
    
   
    gettimeofday(&t2,NULL);
    if (par_id == 0)
    printf("time taken:%d\n",t2.tv_usec-t1.tv_usec);
    close(fd[0]);
    close(fd[1]);
    shm_unlink("list");
    shm_unlink("ready");
  
    munmap(ready,11*sizeof(int));
   



return 0;    
}