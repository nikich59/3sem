#ifndef CLIENT
#define CLIENT

#include "mathserver.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
    int messageQueueId;
    OperationMessage message;
} Client;

ResultMessage getResultFromServer(Client *client);
int initClient(Client *client, char *path);
int getOperation(OperationMessageData *data);
int runClient(Client *client);

void getArray(Client *client, int n) // DEBUG function
{
    int i;
    client->message.msgType = SEND_TO_SERVER_KEY;
    client->message.data.senderId = getpid();
    for (i = 0; i < n; i++)
        msgsnd(client->messageQueueId, &client->message,
               sizeof(client->message.data), 0);

    ResultMessage result;

    for (i = 0; i < n; i++) {
        msgrcv(client->messageQueueId, (ResultMessage *) &result,
                                 sizeof(result.data),
                       getpid() + SEND_TO_SERVER_KEY, 0);
        printf("%lf ", result.data.result);
    }

    printf("\n");
}

ResultMessage getResultFromServer(Client *client)
{
    client->message.msgType = SEND_TO_SERVER_KEY;
    client->message.data.senderId = getpid();
    if (msgsnd(client->messageQueueId, &client->message,
               sizeof(client->message.data), 0) < 0) {
        printf("ERROR: Can\'t send message to server.\n");
    }

    ResultMessage result;

    if (msgrcv(client->messageQueueId, (ResultMessage *) &result,
                         sizeof(result.data),
               getpid() + SEND_TO_SERVER_KEY, 0) < 0) {
        printf("Can\'t recieve result message from server.\n");
    }

    return result;
}

int initClient(Client *client, char *path)
{
    if ((client->messageQueueId = getMessageQueueId(path)) < 0)
        return -1;
}

int getOperation(OperationMessageData *data)
{
    char operationChar;
    scanf("%lf", &data->arg1);
    do {
        scanf("%c", &operationChar);
    } while(operationChar == ' ');
    switch (operationChar) {
    case '+': data->operationCode = SUM_CODE; break;
    case '-': data->operationCode = SUB_CODE; break;
    case '*': data->operationCode = MUL_CODE; break;
    case '/': data->operationCode = DIV_CODE; break;
        default:
        return -1;
    }
    scanf("%lf", &data->arg2);

    return 0;
}

int runClient(Client *client)
{
    while (!getOperation(&client->message.data)) {
        printf("Result is %lf\n", getResultFromServer(client).data.result);
    }

    return 0;
}

#endif // CLIENT

