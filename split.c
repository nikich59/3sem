#include <stdio.h>
#include <string.h>

int split(char *str, char *delim, char ***words, int *count)
{
*words = NULL;
*count = 0;
char *s = strtok(str, delim);
char **use;
int i;
while (s != NULL)
{
use = (char **) malloc((unsigned int)sizeof(char*) * (*count + 1));
for (i = 0; i < *count; i++)
{
use[i] = (*words)[i];
}
if (*words != NULL)
free(*words);
*words = use;
(*words)[*count] = s;

(*count)++;
s = strtok(NULL, delim);
}

return 0;
}

int main ()
{
char **words;
char *delim;
int N = 0;
char *str = (char *) malloc(sizeof(char) * 1000);
delim = (char *) malloc(sizeof(char) * 50);
gets(str);
gets(delim);

split(str, delim, &words, &N);

int i;
printf("\n");
for (i = 0; i < N; i++)
{
printf("%s\n", words[i]);
}

return 0;
}
