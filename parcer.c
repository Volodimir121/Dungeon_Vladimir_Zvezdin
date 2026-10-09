#include <stdio.h>

#include "parser.h"

LineStatus read_line(char* buf, size_t size)
{
    /* Меньше 2 байт — не поместится даже один символ и '\0'. */
    if (buf == NULL || size < 2)
    {
        return LINE_ERROR;
    }

    /* fgets читает не больше size - 1 символов и сама дописывает '\0'.
       В отличие от gets/scanf("%s") переполнить буфер она не может. */
    if (fgets(buf, (int)size, stdin) == NULL)
    {
        buf[0] = '\0';
        return LINE_EOF;
    }

    /* Длина вычисляется один раз проходом до '\n' или '\0'. */
    size_t len = 0;

    while (buf[len] != '\0' && buf[len] != '\n')
    {
        len++;
    }

    if (buf[len] == '\n')
    {
        buf[len] = '\0';
        return LINE_OK;
    }

    /* '\n' в буфере нет. Если буфер заполнен целиком — строка длиннее и
       хвост остался во входном потоке; вычитываем его до конца строки. */
    if (len == size - 1)
    {
        int c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
        }

        return LINE_TOO_LONG;
    }

    /* Иначе это последняя строка файла без '\n' на конце — она целая. */
    return LINE_OK;
}

void to_lower_ascii(char* s)
{
    if (s == NULL)
    {
        return;
    }

    /* Код 'A' + 32 = 'a': заглавные и строчные буквы в ASCII отличаются
       на 'a' - 'A'. Кириллицу и прочие символы не трогаем. */
    for (; *s != '\0'; s++)
    {
        if (*s >= 'A' && *s <= 'Z')
        {
            *s = (char)(*s + ('a' - 'A'));
        }
    }
}

static int is_separator(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

int split_words(char* line, char* words[], int max_words)
{
    if (line == NULL || words == NULL || max_words <= 0)
    {
        return 0;
    }

    int count = 0;
    char* p = line;

    while (count < max_words)
    {
        while (is_separator(*p))
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        words[count++] = p;

        while (*p != '\0' && !is_separator(*p))
        {
            p++;
        }

        /* Конец слова: заменяем разделитель на '\0', чтобы слово
           превратилось в самостоятельную C-строку. */
        if (*p != '\0')
        {
            *p = '\0';
            p++;
        }
    }

    /* Если после max_words слов осталось что-то кроме пробелов — слов
       слишком много. Сообщаем об этом, а не молча отбрасываем хвост. */
    while (is_separator(*p))
    {
        p++;
    }

    if (*p != '\0')
    {
        return -1;
    }

    return count;
}

ParseStatus parse_uint(const char* s, unsigned int max, unsigned int* out)
{
    if (s == NULL || out == NULL || *s == '\0')
    {
        return PARSE_EMPTY;
    }

    unsigned int value = 0;
    int too_big = 0;

    for (const char* p = s; *p != '\0'; p++)
    {
        if (*p < '0' || *p > '9')
        {
            /* Нецифра важнее переполнения: "99999999999x" — это не число. */
            return PARSE_BAD_CHAR;
        }

        if (too_big)
        {
            continue;
        }

        /* Цифра превращается в число вычитанием кода нуля. */
        unsigned int digit = (unsigned int)(*p - '0');

        /* Проверяем, что value * 10 + digit <= max, ДО умножения:
           после переполнения unsigned результат уже не проверить. */
        if (value > max / 10 || (value == max / 10 && digit > max % 10))
        {
            too_big = 1;
            continue;
        }

        value = value * 10 + digit;
    }

    if (too_big)
    {
        return PARSE_TOO_BIG;
    }

    *out = value;
    return PARSE_OK;
}