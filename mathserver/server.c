#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>
#include "mathserver.h"

<<<<<<< HEAD
#define FILE_NAME "temp.txt"
=======
#define MSG_QUEUE_FILE_NAME "msgqueue.txt"

#define SUM_CODE 0
#define SUB_CODE 1
#define MUL_CODE 2
#define DIV_CODE 3
#define POW_CODE 4

#define SEND_TO_SERVER 1

double fsum(double a, double b) {return a + b; }
double fsub(double a, double b) {return a - b; }
double fmul(double a, double b) {return a * b; }
double fdiv(double a, double b) {return a / b; }

const double (*functions[])(double, double) = {fsum, fsub, fmul, fdiv};

typedef struct Data
{
    double firstArgument;
    double secondArgument;
    int operationCode;
    long senderId;
} Data;

typedef struct Message
{
    long msgType;
    Data data;
} Message;

#define maxTaskNum 1024

int currentTaskIndex;
key_t key;
int msqid;
int length;
int canRead;

void *calcInThread(void *v)
{
    /*
     * FIXIT:
     * Операция присваивания не атомарная, поэтому так делать нельзя.
     * Если не хотите делать массив из Message, то надо хотя бы окружить критическую секцию семафорами.
     */
    Message *message = (Message *) v;

    message->data.firstArgument = (*functions[message->data.operationCode])(
                message->data.firstArgument, message->data.secondArgument);
    /*
     * У вас почти написаны только первые 2 пункта упражнения. 
     * Ещё было задание обеспечить, чтобы одновременно выполнялось не более N задач (работало N потоков).
     * Чтобы самим вам убедиться в корректности работы этого пункта, добавьте вызов sleep(,,.) после выполнения арифметической операции.
     * При N = 2 из трех консолей запускаете по клиенту и наблюдаете, что сначала завершаются первые два, а потом только третий.
     */

    printf("Calculated\n");

    /*
     * Это что за переменная такая? Неиспользуемые переменные удаляйте.
     */
    canRead = 0;

    /*
     * Зачем SEND_TO_SERVER нужен? Почему не просто message->data.senderId?
     */
    message->msgType = message->data.senderId + SEND_TO_SERVER;

    if (msgsnd(msqid, message, sizeof(Data), 0) < 0) {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds*)NULL);
        exit(-1);
    }

    free(message);

    return NULL;
}
>>>>>>> 5884aa18481093bf7bd6b4e043cfe979b9169488

int main(void)
{
    Server server;
    if (initServer(&server) < 0) {
        printf("ERROR: Can`t initialize server by \"%s\"\n", FILE_NAME);
        return -1;
    }

<<<<<<< HEAD
    return runServer(&server);
=======
    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
        printf("Can\'t get msqid\n");
        return -1;
    }

    pthread_t threadId;

    int exitCode = 1;
    while (exitCode) {
        message = (Message *) malloc(sizeof(Message));
        if ((length = msgrcv(msqid, (Message *) message,
                             sizeof(Data), SEND_TO_SERVER, 0)) < 0) {
            printf("Can\'t receive message from queue\n");
            return -1;
        }

        printf("Recieved:\nArg1: %f\nArg2: %f\nOperation code: %d\nFrom %li\n",
               message->data.firstArgument,
               message->data.secondArgument,
               message->data.operationCode,
               message->data.senderId);
        
        pthread_create(&threadId, (pthread_attr_t *)NULL,
                               calcInThread, (void *) message);

    }
>>>>>>> 5884aa18481093bf7bd6b4e043cfe979b9169488

    return 0;
}


