#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "client.h"
#include <sys/signal.h>
#include <unistd.h>

int msqId;
key_t key;
const char msgFile[] = "msg.txt";
Client client;

#define SIMPLE_MESSAGE 1
#define CLOSE_MESSAGE  2

void ext()
{
    ClientMessage msg;

    disconnect(&client);

    msg.type = CLOSE_MESSAGE;
    msgsnd(msqId, (void *)&msg, sizeof(Message), 0);

    msgctl(msqId, IPC_RMID, (struct msqid_ds *)NULL);
    exit(0);
}

int runSender()
{
    char s[MESSAGE_LENGTH];
    while (1) {
        printf("%s:", client.nick);
        gets(s);
        for (int i = 0; s[i] != '\0'; i++)
            if (!(s[i] >= 32 || s[i] == '\n' || s[i] == '\t'))
                s[i] = '?';

        printf("\n");
        if (s[0] == ' ') {
            char nick[NICK_LENGTH];
            char msg[MESSAGE_LENGTH];
            int i, j;
            for (i = 1; i < NICK_LENGTH && s[i] != ' '; i++)
                nick[i - 1] = s[i];
            nick[i - 1] = '\0';
            for (j = i; j < MESSAGE_LENGTH && s[j] != '\0';j++)
                msg[j - i] = s[j];
            msg[j - i] = '\0';
            printf("Sending private:\n%s\nto:\n%s\n", msg, nick);
            sendPrivateMessage(&client, msg, nick);
        } else {
            printf("Sending public:\n%s\n", s);
            sendPublicMessage(&client, s);
        }
    }
}

int runReciever()
{
    ClientMessage msg;
    Message serverMsg;
    int size = sizeof(serverMsg);
    while (1) {
        recvfrom(client.socketFd, (void *)&serverMsg, sizeof(serverMsg), 0,
                 &client.server, &size);
        if (serverMsg.type == SERVER_DISCONNECT)
            ext();
        strcpy(msg.message.message, serverMsg.message);
        strcpy(msg.message.reciever, serverMsg.reciever);
        strcpy(msg.message.sender, serverMsg.sender);
        msg.message.type = serverMsg.type;
        msg.type = SIMPLE_MESSAGE;
        msgsnd(msqId, (void *)&msg, sizeof(Message), 0);
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    signal(SIGINT, ext);
    signal(SIGTSTP, ext);
    signal(SIGCHLD, ext);

    printf("Welcome to the LyAlIk.cHaT!!! \n\n");

    if (argc > 1)
        key = atoi(argv[1]);
    else
        key = ftok(msgFile, 0);

    if ((msqId = msgget(key, IPC_CREAT | 0666)) < 0) {
        printf("msgget error.\n");
        ext();
    }

    startClient(&client);

    pid = fork();

    if (pid != 0) {
        runSender();
    } else {
        runReciever();
    }

    ext();
}
