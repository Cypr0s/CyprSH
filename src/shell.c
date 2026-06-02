#include "shell.h"



int32_t main(int32_t argc, char **argv) {
    extern char **environ;
    int32_t file_descriptor = STDIN_FILENO;
    if(argc == 2) {
        open_file(argv[1], O_RDONLY, &file_descriptor);
    }

    HashTable env_table;
    populateEnvTable(&env_table, environ);

    runShell(file_descriptor, &env_table);

    hashTableDtor(&env_table);
    if(file_descriptor != STDIN_FILENO) {
        close(file_descriptor);
    }
    return 0;
}


StatusEnum runShell(int32_t file_descriptor, HashTablePtr env_table) {
    // interactive mode
    if(isatty(file_descriptor)) {
        create_file(HISTORY_FILE_PATH);
        using_history();
        read_history(HISTORY_FILE_PATH);
        char* line = readline("cyprSH> ");
        free(line);
    }

    return SUCCESS;
}
