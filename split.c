#include <stdio.h>
#include <string.h>

/*
 * FIXIT: Куда делось форматирование? Где отступы?
 * Почему вы заменили названия с понятных на свои короткие? Верните tokens_count или words_count вместо своего count.
 * Почему ф-я всегда возвращает 0?
 */

int split(char *str, char *delim, char ***words, int *count)
{
*words = NULL;
*count = 0;
char *s = strtok(str, delim);
char **use;
int i;
while (s != NULL)
{
/*
 * FIXIT:
 * Вы на каждой итерации выделяете и затем освобождаете память. Выделяйте её либо сразу с запасом, определив константу MaxWordsCount, либо по степеням двойки:
 * сначала выделили память под одно слово, если не хватило, то под два realloc`ом, если и этого не хватило, то под 4 и т.д.
 * Примерно так работает структура std::vector в с++
 */
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
/*
 * FIXIT:
 * В коде не должно быть никаких магических чисел вроде 50, 1000.
 * Надо объявить константы MaxStringSize и MaxDelimetersCount и записать в них эти числа. Далее в коде использовать именно эти константы.
 */
char *str = (char *) malloc(sizeof(char) * 1000);
delim = (char *) malloc(sizeof(char) * 50);
gets(str);
gets(delim);

/*
 * FIXIT:
 * Представьте, что хотите оформить вашу ф-ю split в виде библиотеки. Её пользователи будут видеть только сигнатуру ф-и (реализацию они не увидят).
 * Вы выделяете память внутри ф-и, а пользователь должен догадаться не забыть освободить её снаружи. Если вы захотите написать динамическую библиотеку (в винде это dll файлы), то выделить память в
 * библиотеке, а освободить в свой программе у вас просто не получится.
 * Посему надо вынести выделение памяти по слова наружу ф-и split.
 */

split(str, delim, &words, &N);

int i;
printf("\n");
for (i = 0; i < N; i++)
{
printf("%s\n", words[i]);
}

/*
 * FIXIT:
 * Надо чистить самостоятельно за собой память.
 * free(...)
 */

return 0;
}
