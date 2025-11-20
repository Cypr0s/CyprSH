#include "shell.h"

uint32_t main(uint32_t argc, char **argv, char** environ) {
    int32_t file_descriptor = 0; // default stdin
    ExitEnum exit = EXIT_SUCCESS;
    if(argc == 2) {
        exit = open_file(argv[1], O_RDONLY, &file_descriptor);
        if(exit) return exit;
    }

    HashTable env_table; 
    if(populateEnvTable(&env_table, environ)) {
        return EXIT_MALLOC_ERROR;
    }

    run_shell(file_descriptor, &env_table);

    hashTableDispose(&env_table);
    close(file_descriptor);
    return exit;
}