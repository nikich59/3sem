#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>

#define SUCCEED        0
#define CHANGE_BALANCE 1
#define GET_BALANCE    3
#define LACK_OF_MONEY  4
#define WRONG_CLIENT   5
#define CREATE_ACCOUNT 6

typedef struct Message
{
    int command;
    int clientId;
    int money;
} Message;

