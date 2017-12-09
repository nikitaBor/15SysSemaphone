#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "shared.h"

void handle_error(){
  printf("Error: %s\n", strerror(errno));
  exit(1);
}

int main() {
  char *last_line;
  char contents[SHM_SIZE];
  int shm_id, sem_id;
  int fd;
  struct sembuf sbuf = {0, -1, SEM_UNDO};

  printf("Checking for resources...\n");
  
  /* Try to access resources */  
  if((sem_id = semget(SEM_KEY, 0, 0644)) < 0)
    handle_error();

  /* Wait until resource is available */
  if((semop(sem_id, &sbuf, 1)) < 0)
    handle_error();
  
  /* Get shared memory segment */
  if((shm_id = shmget(SHM_KEY, 0, 0644)) < 0)
    handle_error();
  
  /* Access shared memory contents */
  if((last_line = shmat(shm_id, 0, 0)) < 0)
    handle_error();
  
  printf("Last line: %s\n", last_line);
    
  /* Prompt user and update shared memory */
  printf("Your line: ");
  fgets(contents, SHM_SIZE, stdin);

  /* Write to file */
  fd = open(STORY_FILE, O_WRONLY | O_APPEND, 0644);  
  write(fd, contents, strlen(contents));
  close(fd);

  /* Update shared memory segment */
  strncpy(last_line, contents, SHM_SIZE);

  /* Release semaphore */
  sbuf.sem_op = 1;
  if((semop(sem_id, &sbuf, 1)) < 0)
    handle_error();
        
  shmdt(&last_line);
  return 0;
}
