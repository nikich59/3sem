#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <string.h>

int symNum = 0;
int teamNum = 1;
pid_t other;
#define LEN 81

char s[LEN];

void getBit(int n)
{
    if (n == SIGUSR1)
        s[symNum / 8] &= ~(1 << (symNum % 8));
    else
        s[symNum / 8] |=   1 << (symNum % 8);
    symNum++;

    kill(other, SIGINT);
    if (symNum == LEN * 8) {
        printf("%s\n", s);
        exit(0);
    }
}

void sendBit()
{
    char bit = s[symNum / 8] & (1 << symNum % 8);
    symNum++;
    usleep(1);
    if (bit == 0)
        kill(other, SIGUSR1);
    else
        kill(other, SIGUSR2);
    if (symNum == LEN * 8)
        exit(0);
}

void send(char *s)
{
    int i, shift;
    for (i = 0; i < LEN; i++)
        for (shift = 0; shift < 8; shift++) {
            sendBit(s[i] & (1 << shift));
            usleep(10);
        }
}

int main(void)
{
    symNum = 0;
    other = getpid();

    signal(SIGUSR1, getBit);
    signal(SIGUSR2, getBit);
    signal(SIGINT, sendBit);

    pid_t pid = fork();

    if (pid == 0) {

    } else {
        other = pid;
        gets(s);
        sendBit();
    }

    while(1)
        usleep(1);

    return 0;
}
