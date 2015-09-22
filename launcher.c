#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define usleep_delay 100000
#define max_process_str_length 1000
#define max_process_param_num 50
#define process_cfg_delim " "

#define maxStringLength 1000
#define maxDelimStrLength 50
#define maxWordNumber 100
#define max_process_num 50

#define file_in_name "launcher.in"

#define need_wait_default 1
#define need_report_default 1

struct process
{
    time_t launchTime;
    char **argv;
    int need_wait;      // if need wait till the process ends
    int need_report;    // if need report about process' launching to terminal
};

struct process **process_queue;
int process_number = 0;

void split(char *str, char *delim, char ***words, int *words_count);

void add_process(time_t launchTime, char **argv, int need_wait, int need_report);

void remove_process(int num);

void check_launch();

void launch_process(char **argv, int need_wait, int need_report);

void make_process_queue(FILE *inptr);



void split(char *str, char *delim, char ***words, int *words_count)
{
    *words_count = 0;

    char *s = strtok(str, delim);

    while (s != NULL)
    {
        (*words)[*words_count] = s;

        s = strtok(NULL, delim);
        (*words_count)++;
    }

    (*words)[*words_count] = s;

    return;
}

void add_process(time_t launchTime, char **argv, int need_wait, int need_report)
{
    struct process **use = (struct process **) malloc((process_number + 1)
            * sizeof(struct process *));


    int i;
    for (i = 0; i < process_number; i++)
    {
        use[i] = process_queue[i];
    }

    use[process_number] = (struct process*) malloc(sizeof(struct process));

    (use[process_number])->argv = argv;
    (use[process_number])->launchTime = launchTime;
    (use[process_number])->need_wait = need_wait;
    (use[process_number])->need_report = need_report;


    free(process_queue);

    process_queue = use;

    process_number++;
}

void remove_process(int num)
{
    struct process **use = (struct process **) malloc((process_number - 1)
            * sizeof(struct process *));

    int i;
    for (i = 0; i < process_number; i++)
    {
        if (i < num)
        {
            use[i] = process_queue[i];
        }
        if (i > num)
        {
            use[i - 1] = process_queue[i];
        }
    }

    free(process_queue[num]->argv[0]);
    free(process_queue[num]->argv);
    free(process_queue[num]);
    free(process_queue);

    process_queue = use;

    process_number--;
}

void check_launch()
{
    int i;
    time_t currentTime = time(NULL);
    for (i = 0; i < process_number; i++)
    {

        if (process_queue[i]->launchTime <= currentTime)
        {
            launch_process(process_queue[i]->argv, process_queue[i]->need_wait,
                    process_queue[i]->need_report);

            remove_process(i);
        }
    }
}

void launch_process(char **argv, int need_wait, int need_report)
{
    int i;

    if (need_report)
    {
        printf("\nProcess \"%s\" started ", argv[0]);

        if (argv[1] == NULL)
            printf("with no parameters.\n");
        else
        {
            printf("with parmeters:");
            for (i = 1; argv[i] != NULL; i++)
                printf(" \"%s\"", argv[i]);
            printf("\n");
        }
        printf(">>>>>\n");
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(argv[0], argv);
        exit(0);
    }

    int status = 0;
    if (need_wait)
    {
        pid_t child_ID = wait(&status);
        if (need_report)
            printf("Done! %d was returned.\n", status / 0xff);
    }

    if (need_report)
        printf("<<<<<\n");
}

void make_process_queue(FILE *inptr)
{
    char str[max_process_str_length];


    while (fgets(str, max_process_str_length, inptr) != NULL)
    {
        int need_wait = need_wait_default, need_report = need_report_default;

        str[strlen(str) - 1]= '\0';

        int time_end = strcspn(str, process_cfg_delim);

        char *argv_str = (char   *) malloc(max_process_str_length * sizeof(char));

        char **argv = (char **) malloc(max_process_param_num * sizeof(char *));

        int argc;

        strcpy(argv_str, str + time_end + 1);

        split(argv_str, process_cfg_delim, &argv, &argc);

        int process_time = atoi(str) + time(NULL);

        add_process(process_time, argv, need_wait, need_report);
    }
}

int main()
{
    FILE *inptr;

    inptr  = fopen(file_in_name, "r");

    make_process_queue(inptr);

    while (process_number > 0)
    {
        check_launch();
        usleep(usleep_delay);
    }

    fclose(inptr);

    return 0;
}
















