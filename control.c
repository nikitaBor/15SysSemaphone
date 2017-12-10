#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>    
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "shared.h"

void handle_error(){
  printf("Error: %s\n", strerror(errno));
  exit(1);
}

int main(int argc, char *argv[]) {
  int shm_id, sem_id;
  int fd;
  char *command = NULL;
  struct sembuf sbuf = {0, -1, SEM_UNDO};

  if(argc < 2) {
    printf("Error: Command takes additional argument\n");
    return 0;
  } else {
    command = argv[1];
  }
  
  if(!strcmp(command, "-c")) {

    /* Create semaphore */
    union semun su;
    su.val = 1;
    
    if((sem_id = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0644)) < 0)
      handle_error();
    else
      semctl(sem_id, 0, SETVAL, su);    
    if((semop(sem_id, &sbuf, 1)) < 0)
      handle_error();
    
    /* Create shared memory */
    if((shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0644)) < 0)
      handle_error();

    /* Create file */
    if((fd = open(STORY_FILE, O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0)
      handle_error();
    else
      close(fd);
    
  } else if(!strcmp(command, "-v")) {

    int f;
    if(!(f = fork())) {//child
      if(execlp("cat", "cat", STORY_FILE, NULL))
	handle_error();
    } else if (f > 0) {//parent
      int status;
      wait(&status);
    } else {
      handle_error();
    }
    
  } else if(!strcmp(command, "-r")) {
    
    int f;
    if(!(f = fork())) {//child

      /* Read contents of file */
	if((execlp("cat", "cat", STORY_FILE, NULL)) < 0)
	  handle_error();   

    } else if (f > 0){//parent
      int status;
      wait(&status);

      /* Wait until resource is available */
      if((sem_id = semget(SEM_KEY, 0, 0644)) < 0)
        handle_error();
      if(semop(sem_id, &sbuf, 1) < 0)
	handle_error();

      /* Remove shared memory */
      if((shm_id = shmget(SHM_KEY, 0, 0644)) < 0)
	handle_error();
      else
	shmctl(shm_id, IPC_RMID, 0);      

      /* Remove file */
      remove(STORY_FILE);
      printf("------------------------------\nStory file removed\n");

      /* Remove semaphore */
      if(semctl(sem_id, 1, IPC_RMID) < 0)
	handle_error();
      else
	printf("Semaphore removed\n");
    } else {
      
      handle_error();
    }
  }
  
  return 0;
}
