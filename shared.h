#define SEM_KEY 12345
#define SHM_KEY 23456
#define SHM_SIZE 256 * sizeof(char)
#define STORY_FILE "dummy"

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
  struct seminfo *__buf;
};
