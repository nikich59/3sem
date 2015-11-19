#include <stdio.h>
#include "client.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/signal.h>

int msqId;
key_t key;
const char msgFile[] = "msg.txt";

#define SIMPLE_MESSAGE 1
#define CLOSE_MESSAGE  2

void ext()
{
    msgctl(msqId, IPC_RMID, (struct msqid_ds *)NULL);
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, ext);
    signal(SIGTSTP, ext);

    printf("Welcome to the LyAlIk.cHaT viewer!!!\n\n");
    key = 0;
    if (argc > 1)
        key = atoi(argv[1]);
    else
        key = ftok(msgFile, 0);

    printf("Key is %d\n", key);
    msqId = msgget(key, IPC_CREAT | 0666);
    ClientMessage msg;
    while (1) {
        msgrcv(msqId, (void *)&msg, sizeof(msg), 0, 0);
        if (msg.type == CLOSE_MESSAGE)
            ext();
        printMessage(&msg.message);
    }
    return 0;
}
