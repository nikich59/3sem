#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h>
#include "mathserver.h"

#define FILE_NAME "temp.txt"

int main(void)
{
    Server server;
    if (initServer(&server) < 0) {
        printf("ERROR: Can`t initialize server by \"%s\"\n", FILE_NAME);
        return -1;
    }

    return runServer(&server);

    return 0;
}


