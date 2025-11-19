#include <main.h>

int main(uint32_t argc, char **argv, char** environ) {
    int32_t file_descriptor = 0;
    if(argc == 2) {
        file_descriptor = open_file(argv[1], O_RDONLY);
        if(file_descriptor == -1) {
            if (errno == ENOENT)
                return 127;
            if (errno == EACCES)
                return 126;
            return 1;
        }
    }



    return EXIT_SUCCESS;
}