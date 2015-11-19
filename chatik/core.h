#ifndef CORE
#define CORE

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define CONNECTION_REQUEST   1
#define CLIENT_DISCONNECT    2
#define SERVER_DISCONNECT    3
#define PUBLIC_MESSAGE       4
#define PRIVATE_MESSAGE      5
#define CONNECTION_SUCCEED   6
#define WRONG_NICKNAME       7
#define SERVER_MESSAGE       8

#define SERVER_NAME "Server"

#define NICK_LENGTH_MIN 3

typedef struct Message
{
    long type;
    char sender[256];
    char message[1024];
    char reciever[256];
} Message;


#define NICK_LENGTH 256
#define MESSAGE_LENGTH 1024

typedef struct sockaddr_in sockaddr_in;

char serverIp[1024] = "127.0.0.1";

unsigned short getServerPort()
{
    return 51000;
}

const char *getServerIp()
{
    return serverIp;
}

#endif // CORE

