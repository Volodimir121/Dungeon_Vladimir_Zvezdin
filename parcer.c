#include <stdio.h>
#include "parcer.h"

LineStatus read_line(char* buf, size_t size)
{

    if (buf == NULL || size < 2)
    {
        return LINE_ERROR;
    }


    if (fgets(buf, (int)size, stdin) == NULL)
    {
        buf[0] = '\0';
        return LINE_EOF;
    }

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

    if (len == size - 1)
    {
        int c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
        }

        return LINE_TOO_LONG;
    }

    return LINE_OK;
}

void to_lower_ascii(char* s)
{
    if (s == NULL)
    {
        return;
    }

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

        if (*p != '\0')
        {
            *p = '\0';
            p++;
        }
    }

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

            return PARSE_BAD_CHAR;
        }

        if (too_big)
        {
            continue;
        }

        unsigned int digit = (unsigned int)(*p - '0');


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