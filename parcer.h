#ifndef DUNGEON_VLADIMIR_ZVEZDIN_PARSER_H
#define DUNGEON_VLADIMIR_ZVEZDIN_PARSER_H

#include <stddef.h>

/* Размер буфера строки ввода. Туда помещается строка до 62 символов:
   62 символа + '\n' + '\0' = 64. */
#define LINE_BUFFER_SIZE 64

/* Максимум слов в одной введённой строке. */
#define MAX_WORDS 8

typedef enum
{
    LINE_OK,        /* строка прочитана целиком */
    LINE_TOO_LONG,  /* строка не поместилась в буфер, остаток отброшен */
    LINE_EOF,       /* ввод закончился */
    LINE_ERROR      /* неверные параметры вызова */
} LineStatus;

typedef enum
{
    PARSE_OK,
    PARSE_EMPTY,    /* пустая строка или NULL */
    PARSE_BAD_CHAR, /* встретился не цифровой символ */
    PARSE_TOO_BIG   /* число больше допустимого максимума */
} ParseStatus;

/* Читает одну строку из stdin в buf (size — полный размер буфера в байтах).
   Больше size - 1 символов не записывает, строка всегда завершается '\0',
   символ '\n' из неё убирается. Если строка не поместилась, остаток строки
   вычитывается и выбрасывается — иначе он был бы разобран как следующая
   команда. */
LineStatus read_line(char* buf, size_t size);

/* Переводит латинские буквы A-Z в a-z прямо в строке. */
void to_lower_ascii(char* s);

/* Разбивает line на слова по пробелам и табуляциям. Пробелы после слов
   заменяются на '\0' прямо в буфере, в words[i] записывается адрес начала
   i-го слова. Возвращает число слов (0 для пустой строки) или -1, если слов
   больше max_words. Исходная строка line при этом портится — это нормально. */
int split_words(char* line, char* words[], int max_words);

/* Переводит строку из цифр в число, проходя по символам. Допустимы только
   цифры 0-9 (без знака и пробелов). Если число больше max — PARSE_TOO_BIG.
   При любой ошибке *out не изменяется. */
ParseStatus parse_uint(const char* s, unsigned int max, unsigned int* out);

#endif