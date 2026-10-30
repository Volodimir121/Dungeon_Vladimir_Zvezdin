#ifndef ERRORS_H
#define ERRORS_H

typedef enum
{
    ERR_OK = 0,
    ERR_FILE_NOT_FOUND,
    ERR_BAD_FORMAT,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_SIZE
} ErrorCode;

const char* error_code_to_string(ErrorCode code);

#endif /* ERRORS_H */
