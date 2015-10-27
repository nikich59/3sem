#include "client.h"

#define FILE_NAME "temp.txt"

int main(void)
{
    Client client;
    if (initClient(&client, FILE_NAME) < 0) {
        printf("Failed to initialize client by %s\n", FILE_NAME);
        return -1;
    }

    return runClient(&client);
}

