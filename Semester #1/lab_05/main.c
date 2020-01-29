#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdio.h>
#include<stdlib.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
# include <signal.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
/*
Методички нет по 5ой лабе, пример с кодом был на семинаре,
на лекции должны были вам про семафоры рассказать.
 Общее задание звучит так - реализовать алгоритм читатели-писатели на семафорах
(2 считающий и один бинарный, насколько я помню) и разделяемой памяти.
 Дополнительные условия:
1) Не меньше 3х producer-ов и consumers-ов
2) В pipe пишут не менее 2х дочерних процессов
*/

#define AR 0
#define SB 1

#define INC 1
#define DEC -1
#define WAIT 0
#define N 15

#define C_WRITERS 3
#define C_READERS 3


struct sembuf start_write[] = {{AR, WAIT, 0}, {SB, WAIT, 0}, {SB, INC, 0}};
struct sembuf stop_write[] = {{SB, DEC, 0}};
struct sembuf start_read[] = {{SB, WAIT, 0}, {AR, INC, 0}};
struct sembuf stop_read[] = {{AR, DEC, 0}};

int pipe_d[2];

void writer(int *shmaddr, int sem_id, int i)
{
    while(1)
    {
        sleep(rand() % 3);
        printf("\nwriter %d\n", i);
        semop(sem_id, start_write, 3);
        (*shmaddr)++;
        semop(sem_id, stop_write, 1);
    }
    exit(0);
}

void reader(int *shmaddr, int sem_id, int i)
{
    while(1)
    {
        sleep(rand() % 3);
        printf("\nreader %d\n", i);
        semop(sem_id, start_read, 2);
        int cur_value = *shmaddr;
        printf("reader %d\tread value %d\n", i, cur_value);
        semop(sem_id, stop_read, 1);
    }
    exit(0);
}

int main(void)
{
    srand(time(NULL));
    pid_t readers[C_READERS];
    pid_t writers[C_WRITERS];
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|perms);
    int * shm_addr;
    //pipe(pipe_d);
    if (shm_id == -1)
    {
        perror("segment not created\n");
        exit(1);
    }
    int sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT|perms);
    if (sem_id == -1)
    {
        perror("semaphore not created\n");
        exit(2);
    }

    semctl(sem_id, 0, SETVAL, 0);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 0);

    shm_addr =(int *) shmat(shm_id, 0, 0);
    for(int i = 0; i < C_READERS; i++)
    {
        writers[i] = fork();
        if(writers[i] == 0)
        {
            writer(shm_addr, sem_id, i);
        }
        else
            printf("writer id %d\n", writers[i]);
    }
    for(int i = 0; i < C_WRITERS; i++)
    {
        readers[i] = fork();
        if(readers[i] == 0)
        {
            reader(shm_addr, sem_id, i);
        }
        else
            printf("reader id %d\n", readers[i]);
    }

    sleep(10);
    for(int i = 0; i < C_READERS; i++)
    {
        kill(readers[i], SIGINT);
    }
    for(int i = 0; i < C_WRITERS; i++)
    {
        kill(writers[i], SIGINT);
    }

    if(shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        perror("segment could not be destroyed");
        exit(3);
    }
    if(semctl(sem_id, 0, IPC_RMID) == -1)
    {
        perror("semaphore array could not be destroyed");
        exit(4);
    }
    return 0;
}
