#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int num_int, input[200000], num[200000];
int level_1[3] = {-1}, level_2[5] = {-1},
    level_3[9] = {-1}; // boundary of subarray
sem_t semaphore[15], finish, begin;

void *singlethread();
void *multi_1();
void *multi_2(void *label);
void *multi_3(void *label);
void *multi_4(void *label);
int Partition(int p, int rear);
void bubble_sort(int front, int rear);

int main() {
  char filename[30];
  printf("File name : ");
  scanf("%s", &filename);

  FILE *fPtr;
  fPtr = fopen(filename, "r");
  if (!fPtr) {
    printf("fail to open file\n");
    exit(1);
  }

  fscanf(fPtr, "%d", &num_int);
  level_1[2] = num_int;
  level_2[4] = num_int;
  level_3[8] = num_int;

  for (int i = 0; i < num_int; i++) {
    fscanf(fPtr, "%d", &input[i]);
    num[i] = input[i];
  }

  // multithread
  for (int i = 0; i < 15; i++) {
    sem_init(&semaphore[i], 0, 0);
  }
  sem_init(&finish, 0, 0);
  sem_init(&begin, 0, 0);

  pthread_t multi[15];
  int id[15];
  for (int i = 1; i < 15; i++)
    id[i] = i;

  pthread_create(&multi[0], NULL, multi_1, NULL); // level1
  for (int i = 1; i < 3; i++) {
    pthread_create(&multi[i], NULL, multi_2, (void *)&id[i]); // level2
  }
  for (int i = 3; i < 7; i++) {
    pthread_create(&multi[i], NULL, multi_3, (void *)&id[i]); // level3
  }
  for (int i = 7; i < 15; i++) {
    pthread_create(&multi[i], NULL, multi_4, (void *)&id[i]); // level4
  }

  int sec, usec;
  struct timeval start, end;
  gettimeofday(&start, 0);

  sem_post(&semaphore[0]);

  int value = 0;
  // sem_wait(&finish);
  while (value < 8) {
    sem_getvalue(&finish, &value);
    // printf("%d\n", value);
  }

  gettimeofday(&end, 0);
  sec = end.tv_sec - start.tv_sec;
  usec = end.tv_usec - start.tv_usec;
  printf("Multithread lapsed time: %.2f msec\n", sec * 1000 + (usec / 1000.0));

  fPtr = fopen("output1.txt", "w");
  if (!fPtr) {
    printf("fail to open file\n");
    exit(1);
  }

  for (int i = 0; i < num_int; i++) {
    fprintf(fPtr, "%d\n", num[i]);
  }
  fclose(fPtr);

  // singlethread
  for (int i = 0; i < num_int; i++) {
    num[i] = input[i];
  }

  gettimeofday(&start, 0);
  pthread_t single;
  pthread_create(&single, NULL, singlethread, NULL);
  sem_init(&finish, 0, 0);

  gettimeofday(&start, 0);
  sem_post(&begin);
  sem_wait(&finish);

  gettimeofday(&end, 0);
  sec = end.tv_sec - start.tv_sec;
  usec = end.tv_usec - start.tv_usec;
  printf("Singlethread lapsed time: %.2f msec\n", sec * 1000 + (usec / 1000.0));

  fPtr = fopen("output2.txt", "w");
  if (!fPtr) {
    printf("fail to open file\n");
    exit(1);
  }

  for (int i = 0; i < num_int; i++) {
    fprintf(fPtr, "%d\n", num[i]);
  }
  fclose(fPtr);
}

void *multi_1() {
  sem_wait(&semaphore[0]);
  level_1[1] = Partition(0, num_int - 1);
  level_2[2] = level_1[1];
  level_3[4] = level_1[1];

  sem_post(&semaphore[1]);
  sem_post(&semaphore[2]);

  pthread_exit(NULL);
}

void *multi_2(void *label) {
  int *temp = (int *)label, id, i;
  id = *temp;
  i = id - 1;
  sem_wait(&semaphore[id]);

  level_2[2 * i + 1] = Partition(level_1[i] + 1, level_1[i + 1] - 1);
  level_3[2 * (2 * i + 1)] = level_2[2 * i + 1];

  sem_post(&semaphore[2 * id + 1]);
  sem_post(&semaphore[2 * id + 2]);

  pthread_exit(NULL);
}

void *multi_3(void *label) {
  int *temp = (int *)label, id, i;
  id = *temp;
  i = id - 3;
  sem_wait(&semaphore[id]);

  level_3[2 * i + 1] = Partition(level_2[i] + 1, level_2[i + 1] - 1);

  sem_post(&semaphore[2 * id + 1]);
  sem_post(&semaphore[2 * id + 2]);

  pthread_exit(NULL);
}

void *multi_4(void *label) {
  int *temp = (int *)label, id, i;
  id = *temp;
  i = id - 7;
  sem_wait(&semaphore[id]);

  bubble_sort(level_3[i] + 1, level_3[i + 1]);

  sem_post(&finish);

  pthread_exit(NULL);
}

void *singlethread() {
  sem_wait(&begin);

  level_1[1] = Partition(0, num_int - 1);

  for (int i = 0; i < 2; i++) {
    level_2[2 * i] = level_1[i];
    level_2[2 * i + 1] = Partition(level_1[i] + 1, level_1[i + 1] - 1);
  }

  for (int i = 0; i < 4; i++) {
    level_3[2 * i] = level_2[i];
    level_3[2 * i + 1] = Partition(level_2[i] + 1, level_2[i + 1] - 1);
  }

  for (int i = 0; i < 8; i++) {
    bubble_sort(level_3[i] + 1, level_3[i + 1]);
  }

  sem_post(&finish);
  pthread_exit(NULL);
}

int Partition(int p, int rear) {
  if (p >= rear - 1)
    return p; // previous pivot is left most

  int i, j, temp, pivot;
  pivot = num[p];
  i = rear + 1;
  for (j = rear; j > p; j--) {
    if (num[j] >= pivot) {
      i--;
      temp = num[i];
      num[i] = num[j];
      num[j] = temp;
    }
  }

  num[p] = num[--i];
  num[i] = pivot;

  return i;
}

void bubble_sort(int front, int rear) {
  for (int i = front; i < rear - 1; i++) {
    for (int j = i + 1; j < rear; j++) {
      if (num[i] > num[j]) {
        int temp = num[i];
        num[i] = num[j];
        num[j] = temp;
      }
    }
  }
}