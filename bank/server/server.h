#ifndef SERVER
#define SERVER

#include "../bank.h"
#include "pthread.h"

char sempath[] = "sem.txt";

typedef struct Client
{
    int money;
    int id;
} Client;

typedef struct Server
{
    Client *clients;
    int clientNum;
    int min;
    int sockfd;
    int semid;
} Server;

typedef struct ThreadData
{
    Server *server;
    int clientfd;
} ThreadData;

Client *searchClient(const Server *server, int id)
{
    for (int i = 0; i < server->clientNum; i++)
        if (server->clients[i].id == id)
            return server->clients + i;
    return NULL;
}

Client *addClient(Server *server, int id)
{
    Client *use = searchClient(server, id);
    if (use != NULL)
        return use;
    int k = server->clientNum;
    server->clients = (Client *)realloc((void *)server->clients, sizeof(Client)
                                        * (k + 1));
    server->clients[k].id = id;
    server->clients[k].money = 0;
    server->clientNum = k + 1;

    return server + k;
}

int changeBalance(Server *server, int id, int money)
{
    Client *client = searchClient(server, id);
    if (client == NULL)
        return WRONG_CLIENT;
    if (client->money + money >= server->min) {
        client->money += money;
        return SUCCEED;
    } else {
        return LACK_OF_MONEY;
    }
}

void startServer(Server *server)
{
    int port;
    printf("Enter server port:\n");
    scanf("%d", &port);
    server->min = 0;
    server->clients = NULL;
    server->clientNum = 0;

    key_t key;
    if ((key = ftok(sempath, 0)) < 0 ) {
        printf("ftok failed\n");
    }
    if ((server->semid = semget(key, 1, 0666 | IPC_CREAT)) < 0) {
        printf("semget failed\n");
    }
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = 0;
    buf.sem_op = 1;
    semop(server->semid, &buf, 1);

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if ((server->sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket failed:(\n");
    if (bind(server->sockfd, &addr, sizeof(addr)) < 0)
        printf("bind failed:(\n");
    if (listen(server->sockfd, 50) < 0)
        printf("listen failed:(\n");
}

void serverSemOp(Server *server, int n)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = 0;
    buf.sem_op = n;
    semop(server->semid, &buf, 1);
}

void *processClient(void *_data)
{
    struct sembuf buf;
    buf.sem_flg = 0;
    buf.sem_num = 0;
    Message msg;
    ThreadData *data = (ThreadData *)_data;
    while ((read(data->clientfd, &msg, sizeof(msg))) > 0) {
        switch (msg.command) {
        case CHANGE_BALANCE: {
            printf("Got change_balance message by %d, code is: ", msg.clientId);
            serverSemOp(data->server, -1);
            msg.command = changeBalance(data->server, msg.clientId, msg.money);
            serverSemOp(data->server, 1);
            msg.money = 0;
            write(data->clientfd, (void *)&msg, sizeof(msg));
            printf("%d\n", msg.command);
            break;
        }
        case GET_BALANCE: {
            printf("Got get_balance message by %d, code is: ", msg.clientId);
            serverSemOp(data->server, -1);
            Client *client = searchClient(data->server, msg.clientId);
            serverSemOp(data->server, 1);
            if (client == NULL) {
                msg.command = WRONG_CLIENT;
            } else {
                msg.command = SUCCEED;
                msg.money = client->money;
            }
            write(data->clientfd, (void *)&msg, sizeof(msg));
            printf("%d\n", msg.command);
            break;
        }
        case CREATE_ACCOUNT: {
            printf("Got create_account message by %d, code is: ", msg.clientId);
            serverSemOp(data->server, -1);
            Client *client = searchClient(data->server, msg.clientId);
            serverSemOp(data->server, 1);
            if (client == NULL) {
                client = addClient(data->server, msg.clientId);
                msg.command = SUCCEED;
            } else {
                msg.command = WRONG_CLIENT;
            }
            write(data->clientfd, (void *)&msg, sizeof(msg));
            printf("%d\n", msg.command);
            break;
        }
        }
    }
    close(data->clientfd);
    free(data);
    return NULL;
}

int runServer(Server *server)
{
    Message msg;
    struct sockaddr_in clientaddr;
    int clientfd;
    while (1) {
        int clilen = sizeof(clientaddr);
        if ((clientfd = accept(server->sockfd, &clientaddr, &clilen)) < 0)
            printf("accept failed:(\n");
        int id;
        ThreadData *data = (ThreadData *)malloc(sizeof(ThreadData));
        data->server = server;
        data->clientfd = clientfd;
        pthread_create(&id, (pthread_attr_t *)NULL,
                                 processClient, (void *)data);

    }

    return 0;
}

#endif // SERVER

