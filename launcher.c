#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define usleepDelay 100000
#define maxProcessStrLength 1000
#define minProcessStrLength 3
#define maxProcessParamNum 50
#define processCfgDelim " "
#define dontNeedReportChar '$'
#define dontNeedWaitChar '#'
#define commentChar '/'

#define inputFileName "launcher.in"

#define needWaitDefault 1
#define needReportDefult 1

struct Process
{
    time_t launchTime;
    char **argv;
    int needWait;      // if need wait till the process ends
    int needReport;    // if need report about process' launching to terminal
};

struct ProcessQueue
{
    struct Process **processQueue;
    int processNumber;
};

void split(char *str, char *delim, char ***words, int *wordsCount);

void addProcess(struct ProcessQueue *queue, time_t launchTime, char **argv, int needWait, int needReport);

void removeProcess(struct ProcessQueue *queue, int num);

void checkLaunch(struct ProcessQueue *queue);

void launchProcess(char **argv, int needWait, int needReport);

int makeProcessQueue(FILE *inptr, struct ProcessQueue *queue);

int isDigit(char ch)
{
    return (ch >= '0' && ch <= '9');
}

void split(char *str, char *delim, char ***words, int *wordsCount)
{
    *wordsCount = 0;

    char *s = strtok(str, delim);

    while (s != NULL) {
        (*words)[*wordsCount] = s;

        s = strtok(NULL, delim);
        (*wordsCount)++;
    }

    (*words)[*wordsCount] = s;
}

void addProcess(struct ProcessQueue *queue, time_t launchTime, char **argv, int needWait, int needReport)
{
    queue->processQueue = (struct Process **) realloc((void *)queue->processQueue,
                                                      (queue->processNumber + 1) *
                                                      sizeof(struct Process *));

    queue->processQueue[queue->processNumber] = (struct Process *) malloc(sizeof(struct Process *));

    queue->processQueue[queue->processNumber]->argv       = argv;
    queue->processQueue[queue->processNumber]->launchTime = launchTime;
    queue->processQueue[queue->processNumber]->needReport = needReport;
    queue->processQueue[queue->processNumber]->needWait   = needWait;

    queue->processNumber++;
}

void removeProcess(struct ProcessQueue *queue, int num)
{
    struct Process **use = (struct Process **) malloc((queue->processNumber - 1)
            * sizeof(struct Process *));

    int i;
    for (i = 0; i < queue->processNumber; i++) {
        if (i < num) {
            use[i] = queue->processQueue[i];
        }
        if (i > num) {
            use[i - 1] = queue->processQueue[i];
        }
    }

    free(queue->processQueue[num]->argv[0]);
    free(queue->processQueue[num]->argv);
    free(queue->processQueue[num]);
    free(queue->processQueue);

    queue->processQueue = use;

    queue->processNumber--;
}

void checkLaunch(struct ProcessQueue *queue)
{
    int i;

    time_t currentTime = time(NULL);

    for (i = 0; i < queue->processNumber; i++) {
        if (queue->processQueue[i]->launchTime <= currentTime) {
            launchProcess(queue->processQueue[i]->argv, queue->processQueue[i]->needWait,
                    queue->processQueue[i]->needReport);

            removeProcess(queue, i);
        }
    }
}

void launchProcess(char **argv, int needWait, int needReport)
{
    int i;

    if (needReport) {
        printf("\nProcess \"%s\" started ", argv[0]);

        if (argv[1] == NULL) {
            printf("with no parameters.\n");
        } else {
            printf("with parmeters:");
            for (i = 1; argv[i] != NULL; i++)
                printf(" \"%s\"", argv[i]);
            printf("\n");
        }

        printf(">>>>>\n");
    }

    pid_t pid = fork();

    if (pid == 0) {
        int execStatus = execvp(argv[0], argv);

        if (execStatus != 0) {
            printf("\nCan`t launch \"%s\".\n", argv[0]);
        }

        exit(0);
    }

    int status = 0;
    if (needWait) {
        pid_t childId = wait(&status);
        if (needReport)
            printf("Done! Status %d was returned.\n", status / 0xff);
    }

    if (needReport)
        printf("<<<<<\n");
}

int makeProcessQueue(FILE *inptr, struct ProcessQueue *queue)
{
    char *str = (char *) malloc(maxProcessStrLength * sizeof(char));

    while (fgets(str, maxProcessStrLength, inptr) != NULL) {
        if (strlen(str) < minProcessStrLength)
            continue;

        char *argvStr = (char *) malloc(maxProcessStrLength * sizeof(char));
        char **argv = (char **) malloc(maxProcessParamNum * sizeof(char *));

        int needWait = needWaitDefault, needReport = needReportDefult;

        str[strlen(str) - 1] = '\0';

        int timeEnd = strcspn(str, processCfgDelim);

        int argc;
        strcpy(argvStr, str + timeEnd + 1);
        split(argvStr, processCfgDelim, &argv, &argc);

        char *firstDigit = str;
        if (*firstDigit == commentChar)
            continue;

        while ((firstDigit - str) < maxProcessStrLength && !isDigit(*firstDigit)) {
            if (*firstDigit == dontNeedReportChar)
                needReport = 0;
            if (*firstDigit == dontNeedWaitChar)
                needWait = 0;
            if (*firstDigit == ' ')
                return -1;
            firstDigit++;
        }

        int processTime = atoi(firstDigit) + time(NULL);

        addProcess(queue, processTime, argv, needWait, needReport);
    }

    free(str);

    return 0;
}

int main()
{
    FILE *infutFile;

    infutFile  = fopen(inputFileName, "r");

    struct ProcessQueue queue;
    queue.processQueue = NULL;
    queue.processNumber = 0;

    if (makeProcessQueue(infutFile, &queue) != 0) {
        printf("\nCan`t read \"%s\".\n", inputFileName);
    }

    while (queue.processNumber > 0) {
        checkLaunch(&queue);
        usleep(usleepDelay);
    }

    fclose(infutFile);

    return 0;
}
