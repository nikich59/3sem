#include "server.h"
#include <sys/signal.h>
#include <stdlib.h>
#include <string.h>

Server server;

void ext()
{
    clearServer(&server);
    exit(0);
}

int main(void)
{
    signal(SIGINT, ext);
    signal(SIGTSTP, ext);

    initServerDefault(&server);
    return runServer(&server);
}
