#ifndef SERVER
#define SERVER

#include "core.h"

#define FAILED_GET_SOCKET  -1
#define FAILED_BIND        -2
#define DOUBLE_NICK_NAME   -3
#define TOO_SHORT_NICK     -4

typedef struct Client
{
    sockaddr_in address;
    char nick[256];
} Client;

typedef struct Server
{
    int socketFd;
    struct sockaddr_in address;
    Client **clients;
} Server;

int sendPublicMessage(const Server *server, Message *msg)
{
    msg->type = PUBLIC_MESSAGE;
    if (server->clients != NULL) {
        for (int i = 0; server->clients[i] != NULL; i++) {
            sendto(server->socketFd, (void *)msg, sizeof(*msg), 0,
                   &server->clients[i]->address, sizeof(server->clients[i]->address));
        }
    }
    return 0;
}

int sendServerMessage(const Server *server, const char msg[MESSAGE_LENGTH])
{
    Message message;
    message.type = SERVER_MESSAGE;
    strcpy(message.message, msg);
    if (server->clients != NULL) {
        for (int i = 0; server->clients[i] != NULL; i++) {
            sendto(server->socketFd, (void *)&message, sizeof(message), 0,
                   &server->clients[i]->address, sizeof(server->clients[i]->address));
        }
    }
    return 0;
}

int sendClientAdditionMessages(const Server *server, const sockaddr_in *address,
                              const char nick[NICK_LENGTH], int error)
{
    Message msg;
    switch (error) {
    case 0: {
        strcpy(msg.sender, SERVER_NAME);
        msg.type = CONNECTION_SUCCEED;
        sendto(server->socketFd, &msg, sizeof(msg), 0, address,
               sizeof(*address));
        char s[MESSAGE_LENGTH];
        sprintf(s, "%s has just connected to the server.", nick);
        sendServerMessage(server, s);
        printf("%s have just connected to the server.\n", nick);
        break;
    }
    case DOUBLE_NICK_NAME: {
        msg.type = WRONG_NICKNAME;
        strcpy(msg.sender, SERVER_NAME);
        strcpy(msg.message, "Nickname ");
        strcat(msg.message, nick);
        strcat(msg.message, " is already busy. Try another one.");
        sendto(server->socketFd, (void *)&msg, sizeof(msg), 0, address,
               sizeof(*address));
        printf("Nickname %s already exists.\n", nick);
        break;
    }
    case TOO_SHORT_NICK: {
        msg.type = WRONG_NICKNAME;
        strcpy(msg.sender, SERVER_NAME);
        sprintf(msg.message, "Nickname %s is too short. Minimal length is %d.",
                nick, NICK_LENGTH_MIN);
        sendto(server->socketFd, (void *)&msg, sizeof(msg), 0, address,
               sizeof(*address));
        printf("Nickname %s is too short.\n", nick);
        break;

    }
    }
}

int initServerDefault(Server *server)
{
    sockaddr_in address;
    unsigned int ip;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = getServerPort();
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //inet_aton(serverIp, &address.sin_addr);
    return initServer(server, &address, PF_INET, SOCK_DGRAM, 0);
}

int initServer(Server *server, sockaddr_in *address, int domain, int type,
               int protocol)
{
    server->clients = NULL;
    if ((server->socketFd = socket(domain, type, protocol)) < 0)
        return FAILED_GET_SOCKET;
    if (bind(server->socketFd, address, sizeof(*address)) < 0)
        return FAILED_BIND;
    bzero(&server->address, sizeof(server->address));
    server->address.sin_addr.s_addr = address->sin_addr.s_addr;
    server->address.sin_family = address->sin_family;
    server->address.sin_port = address->sin_port;

    return 0;
}

Client *getClient(const Server *server, const char nick[NICK_LENGTH])
{
    if (server->clients != NULL)
        for (int i = 0; server->clients[i] != NULL; i++)
            if (strcmp(server->clients[i]->nick, nick) == 0)
                return server->clients[i];
    return NULL;
}

Client *addClientNick(Server *server, const char nick[NICK_LENGTH],
                      const sockaddr_in *address, int *error)
{
    if (strlen(nick) < NICK_LENGTH_MIN) {
        if (error != NULL)
            *error = TOO_SHORT_NICK;
        return NULL;
    }
    Client *client = NULL;
    int num = 0;
    if (server->clients != NULL) {
        for (num = 0; server->clients[num] != NULL; num++)
            if (strcmp(server->clients[num]->nick, nick) == 0) {
                return server->clients[num];
            }
    }
    client = (Client *)malloc(sizeof(Client));
    strcpy(client->nick, nick);
    client->address = *address;
    server->clients = (Client *)realloc(server->clients, (num + 2) * sizeof(Client *));
    server->clients[num] = client;
    server->clients[num + 1] = NULL;
    return server->clients[num];
}

Client *addClient(Server *server, const Message *msg,
                  const sockaddr_in *address, int *error)
{
    if (error != NULL)
        *error = 0;
    Client *client = addClientNick(server, msg->sender, address, error);
    if (client == NULL) {
        return NULL;
    }
    if (client->address.sin_addr.s_addr != 0 &&
            (client->address.sin_addr.s_addr != address->sin_addr.s_addr ||
            client->address.sin_port != address->sin_port)) {
        if (error != NULL)
            *error = DOUBLE_NICK_NAME;
        return NULL;
    } else {
        return client;
    }
}

int deleteClient(Server *server, char nick[NICK_LENGTH])
{
    if (server->clients != NULL) {
        for (int i = 0; server->clients[i] != NULL; i++)
            if (strcmp(server->clients[i]->nick, nick) == 0) {
                int j;
                free(server->clients[i]);
                for (j = 0; server->clients[j] != NULL; j++)
                    if (j >= i)
                        server->clients[j] = server->clients[j + 1];
                server->clients = (Client **)realloc(server->clients,
                                                     j * sizeof(Client *));
                return 0;
            }
    }
    return -1;
}

int sendClientDisconnectedMessage(const Server *server, const char nick[NICK_LENGTH])
{
    char s[MESSAGE_LENGTH];
    sprintf(s, "%s has just disconnected from the server.", nick);
    sendServerMessage(server, s);
    return 0;
}

void processMessage(Server *server, Message *msg, sockaddr_in *address)
{
    switch (msg->type) {
    case CONNECTION_REQUEST: {
        printf("%s[%s:%u] tries to connect.\n", msg->sender, inet_ntoa(address->sin_addr),
               address->sin_port);
        int err = 0;
        addClient(server, msg, address, &err);
        sendClientAdditionMessages(server, address, msg->sender, err);
        break;
    }
    case CLIENT_DISCONNECT: {
        if (deleteClient(server, msg->sender) == 0) {
            printf("%s[%s:%u] has disconnected.\n", msg->sender, inet_ntoa(address->sin_addr),
                   address->sin_port);
            sendClientDisconnectedMessage(server, msg->sender);
        }
        break;
    }
    case PUBLIC_MESSAGE: {
        Client *client = getClient(server, msg->sender);
        if (client == NULL ||
                client->address.sin_addr.s_addr != address->sin_addr.s_addr ||
                client->address.sin_port != address->sin_port) {
            printf("Unknown user %s tries to send public message\n", msg->sender);
            break;
        }
        sendPublicMessage(server, msg);
        break;
    }
    case PRIVATE_MESSAGE: {
        Client *sender = getClient(server, msg->sender);
        if (sender == NULL ||
                sender->address.sin_addr.s_addr != address->sin_addr.s_addr ||
                sender->address.sin_port != address->sin_port) {
            printf("Unknown user %s tries to send private message\n", msg->sender);
            break;
        }
        Client *reciever = getClient(server, msg->reciever);
        if (reciever != NULL) {
            sendto(server->socketFd, (void *)msg, sizeof(*msg), 0, &sender->address,
                   sizeof(sender->address));
            if (reciever == sender)
                break;
            sendto(server->socketFd, (void *)msg, sizeof(*msg), 0, &reciever->address,
                   sizeof(reciever->address));
        } else {
            Message use;
            use.type = SERVER_MESSAGE;
            sprintf(msg->message, "No such user with nickname %s on the server", msg->reciever);
            sendto(server->socketFd, (void *)msg, sizeof(*msg), 0, &sender->address,
                   sizeof(sender->address));
        }
        break;
    }
    }
}

int runServer(Server *server)
{
    printf("The server has started, port is %u\n", server->address.sin_port);
    Message msg;
    sockaddr_in address;
    int size = sizeof(address);
    while (1) {
        recvfrom(server->socketFd, (void *)&msg, sizeof(msg), 0, &address, &size);
        processMessage(server, &msg, &address);
    }
    return 0;
}

int sendDisconnectMessage(const Server *server)
{
    Message message;
    message.type = SERVER_DISCONNECT;
    if (server->clients != NULL) {
        for (int i = 0; server->clients[i] != NULL; i++) {
            sendto(server->socketFd, (void *)&message, sizeof(message), 0,
                   &server->clients[i]->address, sizeof(server->clients[i]->address));
        }
    }
    return 0;
}

int clearServer(Server *server)
{
    sendDisconnectMessage(server);
    if (server->clients != NULL) {
        for (int i = 0; server->clients[i] != NULL; i++) {
            free(server->clients[i]);
        }
        free(server->clients);
    }
    return 0;
}

#endif // SERVER

