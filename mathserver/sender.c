#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define MSG_QUEUE_FILE_NAME "../server/msgqueue.txt"

#define SUM_CODE 0
#define SUB_CODE 1
#define MUL_CODE 2
#define DIV_CODE 3
#define POW_CODE 4

const char operands[] = {'+', '-', '*', '/', '^'};
const int operandsNumber = 5;

#define SEND_TO_SERVER 1
#define SEND_FROM_SERVER 2

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

int main(void)
{
    int maxTaskNum;
    int currentTaskNum;
    key_t key;
    int msqid;
    int length;

    Message message;

    if ((key = ftok(MSG_QUEUE_FILE_NAME, 0)) < 0) {
        printf("Can\'t generate key\n");
        return -1;
    }

    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0) {
        printf("Can\'t get msqid\n");
        return -1;
    }

    float arg1, arg2;
    char ch;

    while (1) {
        int i;
        scanf("%f", &arg1);
        scanf("%c", &ch);
        for (i = 0; i < operandsNumber; i++)
            if (operands[i] == ch) {
                message.data.operationCode = i;
                break;
            }
        if (i >= operandsNumber)
            break;
        scanf("%f", &arg2);
        message.msgType = SEND_TO_SERVER;
        message.data.firstArgument = arg1;
        message.data.secondArgument = arg2;
        message.data.senderId = getpid();

        if (msgsnd(msqid, (Message *) &message, sizeof(Data), 0) < 0)
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds*) NULL);
            return -1;
        }

        if ((length = msgrcv(msqid, (Message *) &message,
                             sizeof(Data), getpid() + SEND_TO_SERVER, 0)) != sizeof(Data))
        {
            printf("Can\'t receive message from queue\n");
            return -1;
        }

        printf("\nResult is %f\n", message.data.firstArgument);


    }
    return 0;
}



