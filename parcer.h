#ifndef DUNGEON_VLADIMIR_ZVEZDIN_PARSER_H
#define DUNGEON_VLADIMIR_ZVEZDIN_PARSER_H

#include <stddef.h>


#define LINE_BUFFER_SIZE 64

#define MAX_WORDS 8

typedef enum
{
    LINE_OK,
    LINE_TOO_LONG,
    LINE_EOF,
    LINE_ERROR
} LineStatus;

typedef enum
{
    PARSE_OK,
    PARSE_EMPTY,
    PARSE_BAD_CHAR,
    PARSE_TOO_BIG
} ParseStatus;


LineStatus read_line(char* buf, size_t size);

void to_lower_ascii(char* s);


int split_words(char* line, char* words[], int max_words);


ParseStatus parse_uint(const char* s, unsigned int max, unsigned int* out);

#endif