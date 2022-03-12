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
#define MATRIX_DIMENSION_XY 10


int main(int argc, char *argv[]){

char* programstring = (char*)malloc(100);
strcpy(programstring, argv[1]);
//programstring = argv[0];
int progcount = atoi(argv[2]);
char *args[4];

for(int i =0;i<progcount;i++){
    
if (fork()==0){
        
        args[0] =(char*)malloc(100);
        args[1] =(char*) malloc(100);
        args[2] = (char*)malloc(100);
        args[3] = NULL;
        
        sprintf(args[0],"./%s", programstring);
        sprintf(args[1],"%d", i);
        sprintf(args[2],"%d", progcount);
        execv(args[0],args);

        free(args[0]);
        free(args[1]);
        free(args[2]);
        return(0);
    }
    

}
  
free(programstring);
for(int i=0;i<progcount;i++) wait(0);

return(0);

}