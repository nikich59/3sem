#include <stdio.h>
#include <signal.h>
#include "server.h"

void ext()
{
    exit(0);
}

int main(void)
{
    signal(SIGINT, ext);
    signal(SIGTSTP, ext);
    Server server;
    startServer(&server);
    return runServer(&server);
}

