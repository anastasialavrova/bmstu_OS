#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int N = 20;
const int COUNT = 10;
const int PERM = S_IRWXU | S_IRWXG | S_IRWXO;


int* shared_buffer;
int* sh_pos_cons;
int* sh_pos_prod;
int num = 0;

#define SB 0 // бинарный семафор, разграничивает доступ к памяти
#define SE 1 // считывающие семафоры, могут принимать любые значения // количество пустых ячеек 0 - семц закрыт, иначе - открыт
#define SF 2 // считывающие сем, ...; количество заполненных ячеек .. положительное число - колво заполненных ячеек

#define P -1
#define V 1


int fd[2];
char str[50];
char buff[50];
struct sembuf producer_start[2] = { {SE, P, 1}, {SB, P, 1} }; // 1 - тип семафора, 2 - команда, кот. выполняем над ним, 3 - флаг
struct sembuf producer_stop[2] =  { {SB, V, 1}, {SF, V, 1} };
struct sembuf consumer_start[2] = { {SF, P, 1}, {SB, P, 1} };
struct sembuf consumer_stop[2] =  { {SB, V, 1}, {SE, V, 1} };


void producer(const int semid, const int value)
{
    while (1) {
        sleep(1);
        int sem_op_p = semop(semid, producer_start, 2); //Вызов semop() производит операции (неделимые) над выбранными семафорами из набора семафоров
       // (2) - адрес, в котором находится операция, кот. мы будем совершать    (1) - идентификатор массива семафоров   (3) - колво операций
        if ( sem_op_p == -1 )
        {
            perror( "Producer error" );
            exit( 1 );
        }
        
        shared_buffer[num] = num; // в массив разделяемой памяти устанавливаем какое-то значение
        // в num счетчик, с помощью кот мы проходимся по адресному пространству
        
        close( fd[0]);
        printf("%d Producer #%d ----> %d\n",getpid(), value, shared_buffer[num]); // value - id продюсера
        snprintf(str,sizeof(str), "Consumer #%d <---- %d\n", value, shared_buffer[num]); // snprintf направляет данные в символьную строку str
        write(fd[1], str,sizeof(str));
        
        num++;
        
        int sem_op_v = semop(semid, producer_stop, 2);
        if ( sem_op_v == -1 )
        {
            perror( "Produser error" );
            exit( 1 );
        }
    }
}

void consumer(const int semid, const int value)
{
    while(1){
        sleep(2);
        int sem_op_p = semop(semid, consumer_start, 2);
        if ( sem_op_p == -1 )
        {
            perror( "Consumer error" );
            exit( 1 );
        }
        
        close(fd[1]);
        read( fd[0], buff, sizeof(buff));
        printf("%d %s", getpid(), buff);
        
//        (*sh_pos_cons)++;
        int sem_op_v = semop(semid, consumer_stop, 2);
        if ( sem_op_v == -1 )
        {
            perror( "Consumer error" );
            exit( 1 );
        }
    }
}

int main()
{
    int shmid, semid;
    if ((shmid = shmget(IPC_PRIVATE, COUNT * sizeof(int), IPC_CREAT | PERM)) == -1) // shmget - присваивает идентификатор разделяемому сегменту памяти,
        //инициализировали разделяемую память
    {
        perror("Unable to create a shared area.\n");
        exit( 1 );
    }
    
    sh_pos_prod = (int*)shmat(shmid, 0, 0); //Для того чтобы обеспечить доступ к совместно используемой памяти,
    // нужно присоединить ее к адресному пространству процесса. Делается это с помощью функцииshmat; присоединяем разделяемую память

    if (*sh_pos_prod == -1)
    {
        perror("Can't attach memory");
        exit( 1 );
    }
    
    //завершился успешно, он вернет указатель на первый байт совместно используемой памяти

    //
    shared_buffer = sh_pos_prod;
//    sh_pos_cons = sh_pos_prod + sizeof(int);
    
//    (*sh_pos_prod) = 0;
//    (*sh_pos_cons) = 0;
    
    
    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | PERM)) == -1) // Системный вызов semget() возвращает идентификатор набора семафоров, объявили 3 семафора
    {
        perror("Unable to create a semaphore.\n");
        exit( 1 );
    }
    
    int ctrl_sb = semctl(semid, SB, SETVAL, 1); // Вызов semctl выполняет операцию, определённую в cmd, над набором семафоров, назначили начальное значение семафору
    int ctrl_se = semctl(semid, SE, SETVAL, N); // устанавливает к семафору (2) из набора(1), значение (4) .. N пустых ячеек
    int ctrl_sf = semctl(semid, SF, SETVAL, 0); // начальное значение
    
    if ( ctrl_se == -1 || ctrl_sf == -1 || ctrl_sb == -1)
    {
        perror( "Can't set semaphor`s values." );
        exit( 1 );
    }
    
    if (pipe(fd) == -1)
    {
        perror( "Couldn't pipe." );
        return 1;
    }
    
    pid_t pid = -1;
    for (int i = 0; i <  COUNT && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Writer's fork error.\n");
            exit( 1 );
        }
        if (pid == 0)
        {
            producer(semid, i); // указатель на массив семафоров
        }
    }
    for (int i = 0; i < COUNT && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Writer's fork error.\n");
            exit( 1 );
        }
        if (pid == 0)
        {
            consumer(semid, i);
        }
    }
    
    if (pid != 0)
    {
        int status;
        for (int i = 0; i < COUNT*2; ++i)
        {
            wait(&status);
        }
        
        if (shmdt(sh_pos_prod) == -1) // отсоединяем сегмент разделяемой памяти
        {
            perror( "!!! Can't detach shared memory" );
            exit( 1 );
        }
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror( "!!! Can't free memory!" );
            exit( 1 );
        }
    }
}


// особенность семафоров: одной неделимой операцией совершаем действия над всеми ссемафорами
// разделяемая память - участок памяти к которому имеют доступ несколько процессов когда какой-то процесс меняет данные то эти изменения сразу доступны другим процессам
//  несколько процессов не должны находиться там одновременно
// семафоры сигнализируют о том есть ли какой-то процесс в критической секции (критическая секция - там, где разделяемая память)
// shemget, shmat, semop

// средство взаимосисключения в unix системах
