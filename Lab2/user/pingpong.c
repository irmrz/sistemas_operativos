#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>

#define CHILD 0
#define PARENT 1
#define INIT_SEM_VALUE 0

/* Como el enunciado decis devolver 0 e caso de error sigo con la anotacion */
#define ERROR 0
#define SUCCESS 1

int
main(int argc, char *argv[])
{
  int rc,rally;

  // Invalid args
  if(argc != 2){
    fprintf(2, "usage: pingpong rallies...\n");
    exit(ERROR);
  }

  // Parsing args
  rally = atoi(argv[1]);
  if (rally < 1)
  {
    printf("rallie value must be greater than 0...\n");
    exit(ERROR);
  }

  // Open semaphores 
  if ( sem_open(CHILD,INIT_SEM_VALUE) == 0 )
    exit(ERROR); 
  if ( sem_open(PARENT,INIT_SEM_VALUE + 1) == 0) 
    exit(ERROR);
  
  rc = fork();

  //Child
  if (rc == 0) {
    while (rally > 0){

      if (!sem_down(CHILD))
        exit(ERROR); 
    
      printf("\tPong!\n");
      rally--;
      
      if (!sem_up(PARENT))
        exit(ERROR);
    }
    
    exit(SUCCESS);
  }
  else if (rc == -1){
    printf("Error in fork\n");
    exit(ERROR);
  }
  
  // Parent
  else {
    while (rally > 0){
      if (!sem_down(PARENT)) 
        exit(ERROR);
      
      printf("Ping!\n");
      rally--;
      
      if (!sem_up(CHILD)) 
        exit(ERROR);
    }
    wait(NULL);
  }

  // Close semaphores
  sem_close(CHILD);
  sem_close(PARENT);

  exit(SUCCESS);
}