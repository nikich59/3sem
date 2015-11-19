#ifndef CLIENT_H
#define CLIENT_H

#include "core.h"

typedef struct Client
{
    sockaddr_in server;
    char nick[NICK_LENGTH];
    int socketFd;
} Client;

typedef struct ClientMessage
{
    long type;
    Message message;
} ClientMessage;


int printMessage(const Message *msg)
{
    char nick[NICK_LENGTH];
    if (msg->type == WRONG_NICKNAME) {
        strcpy(nick, "[SERVER MESSAGE]");
    }
    if (msg->type == SERVER_MESSAGE) {
        strcpy(nick, "[SERVER MESSAGE]");
    }
    if (msg->type == PRIVATE_MESSAGE) {
        strcpy(nick, "[PRIVATE]");
        strcat(nick, msg->sender);
    }
    if (msg->type == PUBLIC_MESSAGE) {
        strcpy(nick, msg->sender);
    }
    printf("%s: %s\n", nick, msg->message);
    return 0;
}

int sendConnectionRequest(Client *client, Message *msg)
{
    int size = sizeof(client->server);
    strcpy(msg->sender, client->nick);
    msg->type = CONNECTION_REQUEST;
    sendto(client->socketFd, (void *)msg, sizeof(*msg), 0, &client->server,
           sizeof(client->server));
    recvfrom(client->socketFd, (void *)msg, sizeof(*msg), 0, &client->server,
             &size);
    return 0;
}

int initClient(Client *client, const char *nick[NICK_LENGTH], unsigned int ip,
               unsigned short port)
{
    sockaddr_in address;
    bzero((void *)&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(0);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    strcpy(client->nick, nick);
    bzero(&client->server, sizeof(client->server));
    client->server.sin_addr.s_addr = ip;
    client->server.sin_port = port;
    client->server.sin_family = AF_INET;
    if ((client->socketFd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;
    if (bind(client->socketFd, &address, sizeof(address)) < 0)
        return -1;
    return 0;
}

int startClient(Client *client)
{
    char s[1024];
    struct in_addr ip;
    int port;
    printf("Enter server ip:\n");
    gets(s);
    inet_aton(s, &ip);
    printf("Enter server port:\n");
    scanf("%d", &port);
    gets(s);
    initClient(client, client->nick, ip.s_addr, port);
    while (1) {
        printf("Enter your nickname:\n");
        gets(client->nick);
        printf("Connecting to %s:%u with nickname:%s\n",
               inet_ntoa(client->server.sin_addr), client->server.sin_port,
               client->nick);
        Message msg;
        sendConnectionRequest(client, &msg);
        if (msg.type == CONNECTION_SUCCEED)
            break;
        else
            printMessage(&msg);
    }
    printf("OK! Your nickname is %s\n", client->nick);
    return 0;
}

int sendPublicMessage(const Client *client, const char message[MESSAGE_LENGTH])
{
    Message msg;
    msg.type = PUBLIC_MESSAGE;
    strcpy(msg.sender, client->nick);
    strcpy(msg.message, message);
    sendto(client->socketFd, (void *)&msg, sizeof(msg), 0, &client->server,
           sizeof(client->server));
    return 0;
}

int sendPrivateMessage(const Client *client, const char message[MESSAGE_LENGTH],
                       const char reciever[NICK_LENGTH])
{
    Message msg;
    msg.type = PRIVATE_MESSAGE;
    strcpy(msg.sender, client->nick);
    strcpy(msg.message, message);
    strcpy(msg.reciever, reciever);
    sendto(client->socketFd, (void *)&msg, sizeof(msg), 0, &client->server,
           sizeof(client->server));
    return 0;
}

int disconnect(Client *client)
{
    Message msg;
    msg.type = CLIENT_DISCONNECT;
    strcpy(msg.sender, client->nick);
    sendto(client->socketFd, (void *)&msg, sizeof(msg), 0, &client->server,
           sizeof(client->server));
}

#endif // CLIENT_H
