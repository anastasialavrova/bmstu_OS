#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define WRITERS 3
#define READERS 5

#define P -1
#define V 1

#define ACT_READER 0
#define ACT_WRITER 1
#define BIN_ACT_WRITER 2
#define WAIT_WRITER 3


const int PERM =  S_IRWXU | S_IRWXG | S_IRWXO;

struct sembuf start_read[] = {
    { WAIT_WRITER, 0, 1 },
    { ACT_WRITER,  0, 1 },
    { ACT_READER,  V, 1 } };

struct sembuf  stop_read[] = {
    {ACT_READER, P, 1} };

struct sembuf  start_write[] = {
    { WAIT_WRITER,     V, 1 },
    { ACT_READER,      0, 1 },
    { BIN_ACT_WRITER,  P, 1 },
    { ACT_WRITER,      V, 1 },
    { WAIT_WRITER,     P, 1 } };

struct sembuf  stop_write[] = {
    { ACT_WRITER,     P, 1 },
    { BIN_ACT_WRITER, V, 1 }};


void writer(int semid, int* shm, int num)
{
    while (1)
    {
        semop(semid, start_write, 5);
        (*shm)++;  // критическая секция
        printf("process %d   Writer #%d ----> %d\n", getpid(),num, *shm);
        semop(semid, stop_write, 2);
        sleep(2);
    }
}

void reader(int semid, int* shm, int num) {
    
    while (1)
    {
        semop(semid, start_read,3);
        printf("\tprocess %d  Reader #%d <---- %d\n",getpid(), num, *shm);
        semop(semid, stop_read,1);
        sleep(1);
    }
}

int main() {

    int shm_id;
    if ((shm_id = shmget(IPC_PRIVATE, 4, IPC_CREAT | PERM)) == -1)  //выделяем разд. память
    {
        perror("Unable to create a shared area.\n");
        exit( 1 );
    }
    
    int *shm_buf = (int*)shmat(shm_id, 0, 0);  // получаем ее адрес
    if (shm_buf == (void*) -1)
    {
        perror("Can't attach memory");
        exit( 1 );
    }
    
    (*shm_buf) = 0;
    
    int sem_id;
    if ((sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | PERM)) == -1)  // 4 набора семафоров
    {
        perror("Unable to create a semaphore.\n");
        exit( 1 );
    }
    
    int ctrl = semctl(sem_id, BIN_ACT_WRITER, SETVAL, 1); // устанавливает значения
    if ( ctrl == -1)
    {
        perror( "Can't set semaphor`s values." );
        exit( 1 );
    }
    
    pid_t pid = -1;
    
    for (int i = 0; i < WRITERS && pid != 0; i++) // инициализируем writers
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Writer's fork error.\n");
            exit( 1 );
        }
        if (pid == 0)  // если дочерний процесс, то запускаем функцию писателей
        {
            writer(sem_id, shm_buf, i);
        }
    }
    
    for (int i = 0; i < READERS && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Reader's fork error.\n");
            exit( 1 );
        }
        if (pid == 0)
        {
            reader(sem_id, shm_buf, i);
        }
    }
    
    if (shmdt(shm_buf) == -1)
    {
        perror( "Can't detach shared memory" );
        exit( 1 );
    }
    
    if (pid != 0)
    {
        int *status;
        for (int i = 0; i < WRITERS + READERS; ++i) // ждем завершение всех читателей и писателей
        {
            wait(status);
        }
        if (shmctl(shm_id, IPC_RMID, NULL) == -1)
        {
            perror( "Can't free memory!" );
            exit( 1 );
        }
    }
    
    return 0;
}
