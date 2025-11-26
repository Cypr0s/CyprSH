#include "shell.h"



uint32_t main(uint32_t argc, char **argv, char** environ) {
    int32_t file_descriptor = 0; // default stdin
    if(argc == 2) {
        open_file(argv[1], O_RDONLY, &file_descriptor);

        if(error) return error;
    }

    HashTable env_table; 
    populateEnvTable(&env_table, environ);
    if(error) {
        print_error(); // TODO create ERROR func
        return error;
    }
        

    run_shell(file_descriptor, &env_table);
    if(error) {
        print_error(); // TODO create ERROR func
        return error;
    }

    hashTableDispose(&env_table);
    close(file_descriptor);
    return error;
}


void run_shell(uint32_t file_descriptior, HashTablePtr env_table) {
    // execute mode
    if(!isatty(file_descriptior)) {
        exec();
        return;
    }

    // load history file or create it if it does not exist
    create_file(HISTORY_FILE_PATH);
    using_history();
    read_history(HISTORY_FILE_PATH);

}