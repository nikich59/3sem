#include <stdio.h>
#include <string.h>

#define maxStringLength 1000
#define maxDelimStrLength 50
#define maxWordNumber 100


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

    return 0;
}

int main ()
{
    int wordNum = 0;
    char *str = (char *) malloc(sizeof(char) * maxStringLength);
    char **words = (char **) malloc(sizeof(char *) * maxWordNumber);
    char *delim = (char *) malloc(sizeof(char) * maxDelimStrLength);

    gets(str);

    gets(delim);

    split(str, delim, &words, &wordNum);

    int i;
    for (i = 0; i < wordNum; i++)
    {
        printf("%s\n", words[i]);
    }

    free(delim);
    free(str);
    free(words);

    return 0;
}
