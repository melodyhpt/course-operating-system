#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

int num_int, wait_front, wait_rear, input[200000], num[200000], last = 0;
int level_1[3] = {-1}, level_2[5] = {-1},
    level_3[9] = {-1};  // boundary of subarray
int work[8];  // thread_id <-> job_id, work[i] == 0 if ith thread is not working
int waiting[15];  // job_id
sem_t semaphore[8], finish, begin, empty;
pthread_mutex_t write_job;

void *distribute();
void *workThread(void *label);
int Partition(int p, int rear);
void bubble_sort(int front, int rear);

int main() {
  // read input
  FILE *fPtr;
  fPtr = fopen("input.txt", "r");
  if (!fPtr) {
    printf("fail to open file\n");
    exit(1);
  }
  fscanf(fPtr, "%d", &num_int);
  for (int i = 0; i < num_int; i++) fscanf(fPtr, "%d", &input[i]);

  // initialize variable
  level_1[2] = num_int;
  level_2[4] = num_int;
  level_3[8] = num_int;

  for (int j = 0; j < 8; j++) sem_init(&semaphore[j], 0, 0);
  // sem_init(&finish, 0, 0);
  sem_init(&begin, 0, 0);
  sem_init(&empty, 0, 1);
  pthread_mutex_init(&write_job, NULL);

  // create job distribute thread
  pthread_t job_thread;
  pthread_create(&job_thread, NULL, distribute, NULL);

  // create thread pool
  int id[8];
  pthread_t thread[8];
  for (int i = 0; i < 8; i++) id[i] = i;
  for (int i = 0; i < 8; i++)
    pthread_create(&thread[i], NULL, workThread, (void *)&id[i]);

  for (int i = 1; i <= 8; i++) {
    // initialize variables
    for (int j = 0; j < num_int; j++) num[j] = input[j];

    waiting[0] = 1;
    wait_front = 0;
    wait_rear = 1;
    for (int j = 0; j < 8; j++) work[j] = 0;
    if(i == 8) last = 1;

    sem_init(&empty, 0, i);
    sem_init(&finish, 0, 0);

    // do sorting
    int sec, usec;
    struct timeval start, end;
    gettimeofday(&start, 0);

    sem_post(&begin);

    int value = 0;
    while (value < 8) {
      sem_getvalue(&finish, &value);
      //   printf("%d\n", value);
    }

    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("%d thread : %.2f msec\n", i, sec * 1000 + (usec / 1000.0));

    // print result to file
    char filename[20] = {}, temp[2];
    temp[0] = i +'0';
    temp[1] = '\0';
    strcpy(filename, "output_");
    strcat(filename, temp);
    strcat(filename, ".txt");

    fPtr = fopen(filename, "w");
    if (!fPtr) {
      printf("fail to open file\n");
      exit(1);
    }

    for (int i = 0; i < num_int; i++) {
      fprintf(fPtr, "%d ", num[i]);
    }
    fclose(fPtr);
  }
}

void *distribute() {
  for(int i=0; i<8; i++) {
    sem_wait(&begin);
    while (wait_front < 15) {
      if (wait_front < wait_rear) {
        sem_wait(&empty);
        // printf("find empty thread\n");

        pthread_mutex_lock(&write_job);
        int thread_id;  // find free thread
        for (int i = 0; i < 8; i++) {
          if (work[i] == 0) {
            thread_id = i;
            break;
          }
        }

        // assign job to thread
        work[thread_id] = waiting[wait_front];
        wait_front++;
        sem_post(&semaphore[thread_id]);
        pthread_mutex_unlock(&write_job);
        // printf("%d posted\n", thread_id);
      }
    }
  }
  pthread_exit(NULL);
}

void *workThread(void *label) {
  int *temp = (int *)label, id, job_id;
  id = *temp;
  while (1) {
    sem_wait(&semaphore[id]);
    job_id = work[id];
    // printf("%d %d\n", id, job_id);

    // do job
    if (job_id == 1) {
      level_1[1] = Partition(0, num_int - 1);
      // printf("%d %d %d\n", job_id, 0, num_int - 1);
      level_2[2] = level_1[1];
      level_3[4] = level_1[1];

    } else if (job_id >= 2 && job_id <= 3) {
      int i = job_id - 2;
      level_2[2 * i + 1] = Partition(level_1[i] + 1, level_1[i + 1] - 1);
      // printf("%d %d %d\n", job_id, level_1[i] + 1, level_1[i + 1] - 1);
      level_3[2 * (2 * i + 1)] = level_2[2 * i + 1];

    } else if (job_id >= 4 && job_id <= 7) {
      int i = job_id - 4;
      level_3[2 * i + 1] = Partition(level_2[i] + 1, level_2[i + 1] - 1);
      // printf("%d %d %d\n", job_id, level_2[i] + 1, level_2[i + 1] - 1);
    } else if (job_id >= 8 && job_id <= 15) {
      int i = job_id - 8;
      bubble_sort(level_3[i] + 1, level_3[i + 1]);
      // printf("%d %d %d\n", job_id, level_3[i] + 1, level_3[i + 1] - 1);
      sem_post(&finish);
      if(last == 1) break;
    }

    // add next jobs
    pthread_mutex_lock(&write_job);
    if (job_id < 8) {
      waiting[wait_rear] = 2 * job_id;
      waiting[wait_rear + 1] = 2 * job_id + 1;
      wait_rear += 2;
    }

    // clear thread
    work[id] = 0;
    sem_post(&empty);
    pthread_mutex_unlock(&write_job);
  }

  pthread_exit(NULL);
}

int Partition(int p, int rear) {
  if (p >= rear - 1) return p;  // previous pivot is left most

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