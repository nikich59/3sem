#ifndef MATHSERVER
#define MATHSERVER

#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

#define SUM_CODE 0
#define SUB_CODE 1
#define MUL_CODE 2
#define DIV_CODE 3

#define DEFAULT_MAX_THREAD_NUM 5

#define SEND_TO_SERVER_KEY 1

#define SEM1_PATH "sem1.txt"
#define SEM2_PATH "sem2.txt"
#define SEM_THREAD_PATH "sem3.txt"
#define MSG_PATH "temp.txt"

typedef struct OperationMessageData
{
    double arg1;
    double arg2;
    unsigned int operationCode;
    long senderId;
} OperationMessageData;

typedef struct OperationMessage
{
    long msgType;
    OperationMessageData data;
} OperationMessage;

typedef struct ResultMessageData
{
    double result;
} ResultMessageData;

typedef struct ResultMessage
{
    long msgType;
    ResultMessageData data;
} ResultMessage;

typedef struct Server
{
    int maxThreadNum;
    int currentThreadNum;
    int messageQueueId;
    int atomicSemaphoreId;
    int threadNumsemaphoreId;
    OperationMessage message;
} Server;

typedef struct
{
    Server *server;
    OperationMessage message;
} ThreadData;

void *calculate(void *v);
int getMessageQueueId(char *fileName);
int runServer(Server *server);
int getSemaphoreId(char *pathName);
int startAtomic(Server *server);
void finishAtomic(Server *server);
int initServer(Server *server);
void startComputingInThread(Server *server);

void *calculate(void *v)
{
    printf("Started computing\n");
    ThreadData *threadData = (ThreadData *) v;

    finishAtomic(threadData->server);

    ResultMessage resultMessage;

    double arg1 = threadData->message.data.arg1;
    double arg2 = threadData->message.data.arg2;
    double result;

    switch (threadData->message.data.operationCode) {
    case SUM_CODE: result = arg1 + arg2; break;
    case SUB_CODE: result = arg1 - arg2; break;
    case MUL_CODE: result = arg1 * arg2; break;
    case DIV_CODE: result = arg1 / arg2; break;
    default:
        result = 0.0;
    }

    resultMessage.data.result = result;
    resultMessage.msgType = threadData->message.data.senderId +
            SEND_TO_SERVER_KEY;

    if (msgsnd(threadData->server->messageQueueId, &resultMessage,
               sizeof(resultMessage.data), 0) < 0) {
        printf("ERROR: Can\'t send message to queue\n");
    }

    printf("Ended computing\n");
    struct sembuf semaphore;

    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op  = 1;
    semop(threadData->server->threadNumsemaphoreId, &semaphore, 1);

    startAtomic(threadData->server);
    threadData->server->currentThreadNum--;
    finishAtomic(threadData->server);

    free(threadData);

    return NULL;
}

int getMessageQueueId(char *fileName)
{
    key_t key;
    int msqid;
    if ((key = ftok(fileName, 0)) < 0) {
        return -1;
    }
    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
        return -1;
    }
    return msqid;
}

int runServer(Server *server)
{
    int exitCode = 1;
    while (exitCode) {
        if (msgrcv(server->messageQueueId, (OperationMessage *) &server->message,
                             sizeof(server->message.data), SEND_TO_SERVER_KEY, 0) < 0) {
            printf("Can\'t receive message from queue\n");
            usleep(1);
            continue;
        }

        printf("Recieved:\nArg1: %f\nArg2: %f\nOperation code: %d\nFrom %li\n",
               server->message.data.arg1,
               server->message.data.arg2,
               server->message.data.operationCode,
               server->message.data.senderId);

        startComputingInThread(server);
    }
}

int getSemaphoreId(char *pathName)
{
    key_t key;
    if ((key = ftok(pathName, 0)) < 0 ) {
        return -1;
    }
    return semget(key, 1, 0666 | IPC_CREAT);
}

int startAtomic(Server *server)
{
    struct sembuf semaphore;
    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op = -1;
    return semop(server->atomicSemaphoreId, &semaphore, 1);
}

void finishAtomic(Server *server)
{
    struct sembuf semaphore;
    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op  = 1;
    semop(server->atomicSemaphoreId, &semaphore, 1);
}

int initServer(Server *server)
{
    server->messageQueueId = getMessageQueueId(MSG_PATH);
    if (server->messageQueueId < 0)
        return -1;
    server->atomicSemaphoreId = getSemaphoreId(SEM1_PATH);
    if (server->atomicSemaphoreId < 0)
        return -1;
    server->threadNumsemaphoreId = getSemaphoreId(SEM2_PATH);
    if (server->threadNumsemaphoreId < 0)
        return -1;
    server->currentThreadNum = 0;
    server->maxThreadNum = DEFAULT_MAX_THREAD_NUM - 1;

    struct sembuf semaphore;
    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op  = 1;
    semop(server->atomicSemaphoreId, &semaphore, 1);

    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op  = server->maxThreadNum;
    semop(server->threadNumsemaphoreId, &semaphore, 1);

    return 0;
}

void startComputingInThread(Server *server)
{
    startAtomic(server);
    ThreadData *threadData = (ThreadData *) malloc(sizeof(ThreadData));
    threadData->server = server;
    threadData->message.data.arg1 = server->message.data.arg1;
    threadData->message.data.arg2 = server->message.data.arg2;
    threadData->message.data.operationCode = server->message.data.operationCode;
    threadData->message.data.senderId = server->message.data.senderId;

    struct sembuf semaphore;
    semaphore.sem_flg = 0;
    semaphore.sem_num = 0;
    semaphore.sem_op = -1;
    semop(threadData->server->threadNumsemaphoreId, &semaphore, 1);

    pthread_t threadId;
    pthread_create(&threadId, (pthread_attr_t *)NULL, calculate, (void *) threadData);
}

#endif // MATHSERVER