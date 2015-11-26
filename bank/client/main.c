#include <stdio.h>
#include "../bank.h"

void ext()
{
    exit(0);
}

int main(void)
{
    signal(SIGINT, ext);
    signal(SIGTSTP, ext);
    int sockfd;
    Message msg;
    int id;
    struct sockaddr_in addr;
    char ip[1024];
    int port;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket failed:(\n");
    }
    printf("Welcome to bank client!\n");
    do {
        printf("Enter server ip:\n");
        scanf("%s", ip);
    } while (inet_aton(ip, &addr.sin_addr) < 0);
    printf("Enter server port:\n");
    scanf("%d", &port);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (connect(sockfd, &addr, sizeof(addr)) < 0)
        printf("connect failed:(\n");
    while (1) {
        printf("Enter bank account id:\n");
        scanf("%d", &id);
        msg.clientId = id;
        msg.money = 0;
        msg.command = GET_BALANCE;
        write(sockfd, (void *)&msg, sizeof(msg));
        read(sockfd, (void *)&msg, sizeof(msg));
        if (msg.command == SUCCEED) {
            printf("Succeed!\n");
            break;
        }
        if (msg.command == WRONG_CLIENT) {
            printf("Wrong client\n");
            printf("Want to create?(y/n)\n");
            char ch;
            scanf("%c", &ch);
            scanf("%c", &ch);
            if (ch == 'y') {
                msg.command = CREATE_ACCOUNT;
                write(sockfd, (void *)&msg, sizeof(msg));
                read(sockfd, (void *)&msg, sizeof(msg));
                break;
            }
        }
    }

    printf("Your balance is %d.\n", msg.money);

    while (1) {
        int money;
        printf("\n\nEnter money:\n");
        scanf("%d", &money);
        msg.clientId = id;
        msg.money = money;
        msg.command = CHANGE_BALANCE;
        write(sockfd, (void *)&msg, sizeof(msg));
        read(sockfd, (void *)&msg, sizeof(msg));
        if (msg.command == SUCCEED)
            printf("Succeed!\n");
        if (msg.command == LACK_OF_MONEY)
            printf("Lack of money.\n");
        msg.command = GET_BALANCE;
        write(sockfd, (void *)&msg, sizeof(msg));
        read(sockfd, (void *)&msg, sizeof(msg));
        printf("You have %d on your account #%d", msg.money, id);
    }

    return 0;
}

