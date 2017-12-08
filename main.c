#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>    
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define SHM_KEY 23456
#define SEM_KEY 12345

void print_error(){
  printf("Error: %s\n", strerror(errno));
}

int main(int argc, char *argv[]) {
  int shm_id;
  int sem_id, sem_val;
  int fd;
  char *command = argv[1];
  
  if(!strcmp(command, "-c")) {
    /* Create shared memory */
    if((shm_id = shmget(SHM_KEY, 256 * sizeof(char), IPC_CREAT | IPC_EXCL | 0644)) < 0)
      print_error();

    /* Create semaphore */
    if((sem_id = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0600)) < 0)
      print_error();

    /* Create file */
    if((fd = open("dummy", O_CREAT | O_EXCL | O_TRUNC)) < 0)
      print_error();
    else
      close(fd);

    if(sem_id) {
      semctl(sem_id, 0, SETVAL, 1);      
    }
      
    //printf("Semaphore created: %d\n", sem_id);
    //printf("Value set: %d\n", sem_val);      
  } else if(!strcmp(command, "-v")) {
  
    if((fd = open("dummy", O_RDONLY)) < 0)
      print_error();

    /* REMEMBER TO INCREASE THE AMOUNT OF SIZE FOR READ */
    int nums_read = read(fd, stdout, 1024);
    printf("Chars read: %d\n", nums_read);

    close(fd);
  
  } else if(!strcmp(command, "-r")) {    

    if(!fork()) {
      execlp("./control", "./control", "-v");
    } else {
      int status;
      wait(&status);
      
      if((sem_id = semget(SEM_KEY, 0, 0600)) < 0)
	print_error();    

      if(sem_id >= 0) {
	/* Remove semaphore */
	if(semctl(sem_id, 1, IPC_RMID) < 0)
	  print_error();
	else {
	  printf("Semaphore removed\n");
	}
      }

      
      
    }
  }
  
  return 0;
}
