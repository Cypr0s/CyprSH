#include <errno.h>



typedef enum return_values {
    SUCCESS=0,
    ERROR_DEFAULT=1,
    ERROR_SHELL_MISUSE=2,
    ERROR_MALLOC_FAILURE=3,
    ERROR_INT_OVERFLOW=4,
    ERROR_INDEX_OUT_OF_BOUNDS=5,
    ERROR_COMM_CANNOT_EXEC=126,
    ERROR_COMMAND_NOT_FOUND=127,
} StatusEnum;

void print_errno(const char *path);

void print_error(void);