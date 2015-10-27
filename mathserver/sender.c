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
        
        /*
         * FIXIT:
         * Есть стандартные ф-и поиска символа в строке и т.д.
         * http://cppstudio.com/cat/309/325/
         * Не нужно писать свои.
         */

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

        /*
         * Непонятно, зачем нужна константа SEND_TO_SERVER.
         */
        if ((length = msgrcv(msqid, (Message *) &message,
                             sizeof(Data), getpid() + SEND_TO_SERVER, 0)) != sizeof(Data))
        {
            printf("Can\'t receive message from queue\n");
            return -1;
        }

        /*
         * Можно было бы сделать разные структуры дл отправки запроса и для получения результата.
         * Можно только догадываться, что сервер положит результат произведения в firstArgument.
         * 
         * А чтобы не дублировать дважды определения этих структур и кучу констант в клиенте и сервере их можно вынести в отдельный .h файл.
         * Чтобы скомпилировать несколько файлов досточно написать их через пробел:
         * gcc client.c mytypes.h -o client
         */
        printf("\nResult is %f\n", message.data.firstArgument);
    }
    return 0;
}



