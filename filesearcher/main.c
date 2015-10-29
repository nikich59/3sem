#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_FILE_NAME_LENGTH 256
#define MAX_PATH_LENGTH  2048
#define MAX_SEARCH_DEPTH 10

int search(char *fileName, char *dirName, int depth)
{
    if (depth < 0)
        return -1;
    DIR *dir;
    if ((dir = opendir(dirName)) == NULL)
        return -1;
    struct dirent *use;
    struct stat Stat;
    char *end = strchr(dirName, '\0');

    while ((use = readdir(dir)) != NULL) {
        if (strcmp(fileName, use->d_name) == 0) {
            dirName = strcat(dirName, "/");
            dirName = strcat(dirName, use->d_name);
            return 0;
        }
        stat(use->d_name, &Stat);
        if (/*S_ISDIR(Stat.st_mode) && */strcmp(use->d_name, ".") != 0 &&
                strcmp(use->d_name, "..") != 0) {
            dirName = strcat(dirName, "/");
            dirName = strcat(dirName, use->d_name);
            if (search(fileName, dirName, depth - 1) >= 0) {
                return 0;
            }
        }
        *end = '\0';
    }

    return -1;
}

void ls(char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
        return;
    struct dirent *_dirent;
    printf("                name ");
    printf("                type ");
    printf("         size, bytes\n\n");
    struct stat buf;
    while((_dirent = readdir(dir)) != NULL) {
        stat(_dirent->d_name, &buf);
        printf("%20s %20s %20u\n", _dirent->d_name,
               S_ISREG(buf.st_mode) ? "reg" : "dir", (unsigned int)buf.st_size);
    }
}

int printFileMap(char *dirName, int base)
{
    const int step = 2;
    const int columnNum = 70;
    DIR *dir;
    if ((dir = opendir(dirName)) == NULL)
        return 0;
    struct dirent *use;
    struct stat Stat;
    char *end = strchr(dirName, '\0');

    while ((use = readdir(dir)) != NULL) {
        if (strcmp(use->d_name, ".") == 0 || strcmp(use->d_name, "..") == 0)
            continue;
        stat(use->d_name, &Stat);
        dirName = strcat(dirName, "/");
        dirName = strcat(dirName, use->d_name);
        for (int i = 0; i < base; i++)
            printf(" ");
        printf("%s", use->d_name);
        for (int i = base + strlen(use->d_name);
             i < columnNum - (Stat.st_size == 0 ? 0 :
                              (int)trunc(log10((double) Stat.st_size))); i++)
            printf("_");

        printf("%u\n", Stat.st_size);
        printFileMap(dirName, base + step);
        *end = '\0';
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char s[MAX_FILE_NAME_LENGTH];
    char path[MAX_PATH_LENGTH] = ".";

    int maxDepth = MAX_SEARCH_DEPTH;
    if (argc > 1) {
        strcpy(s, argv[1]);
        if (argc > 2)
            maxDepth = atoi(argv[2]);
    } else {
        printf("Enter name of file to be searched:\n");
        scanf("%s", s);
    }

    if (search(s, path, maxDepth) >= 0)
        printf("%s\n", path);
    else
        printf("No such file or directory.\n");

    return 0;
}

