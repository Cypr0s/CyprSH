#include "shell.h"



uint32_t main(uint32_t argc, char **argv, char** environ) {
    int32_t file_descriptor = 0; // default stdin
    if(argc == 2) {
        open_file(argv[1], O_RDONLY, &file_descriptor);

        
    }

    HashTable env_table; 
    populateEnvTable(&env_table, environ);
        

    run_shell(file_descriptor, &env_table);

    hashTableDispose(&env_table);
    close(file_descriptor);
    return 0;
}


StatusEnum run_shell(int32_t file_descriptor, HashTablePtr env_table) {
    // non-execute mode
    if(isatty(file_descriptor)) {
        create_file(HISTORY_FILE_PATH);
        using_history();
        read_history(HISTORY_FILE_PATH);
        readline("cyprSH>");
    }
    fdopen();

    // load history file or create it if it does not exist
}