#include "errors.h"

const char* error_code_to_string(ErrorCode code)
{

    switch (code)
    {
    case ERR_OK:
        return "Success.";
    case ERR_FILE_NOT_FOUND:
        return "Error: File not found.";
    case ERR_BAD_FORMAT:
        return "Error: Invalid or corrupted level format.";
    case ERR_OUT_OF_MEMORY:
        return "Error: Memory allocation failed (out of memory).";
    case ERR_INVALID_SIZE:
        return "Error: Invalid map size (must be at least 3x3).";
    default:
        return "Error: Unknown error.";
    }

}
