/**
 * @file        shell.c
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Shell execution modes: interactive, script, and string
 */

#include "shell.h"

/** @brief Main entry point for shell
 *  @param argc Argument count
 *  @param argv Command line arguments
 *  @return Exit status code
 */
int32_t main(int32_t argc, char **argv) {
    extern char **environ;

    // create env table
    HashTable env_table;
    populateEnvTable(&env_table, environ);

    StatusEnum st = SUCCESS;

    // choose run mode / script / string / interactive / pipe
    if(argc >= 3 && streq(argv[1], "-c")) {
        st = runString(argv[2], &env_table);
    } else if(argc == 2) {
        int32_t fd = STDIN_FILENO;
        st = openFile(argv[1], O_RDONLY, &fd);

        if(st != SUCCESS) {
            hashTableDtor(&env_table);
            return st;
        }

        FILE* input = fdopen(fd, "r"); // create file from fd
        if(input == NULL) {
            printError("main", "Cannot open file: %s", argv[1]);
            close(fd);
            hashTableDtor(&env_table);
            return 1;
        }
        st = runScript(input, &env_table);
        fclose(input);

    } else if(isatty(STDIN_FILENO)) {
        st = runInteractive(&env_table);
    } else {
        st = runScript(stdin, &env_table);
    }


    hashTableDtor(&env_table);
    return 0;
}


StatusEnum runInteractive(HashTablePtr env_table) {
    // loading history
    char history_path[PATH_MAX] = {0};
    char* home = getenv("HOME");
    if(home != NULL) {
        snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE_NAME);
    } else {
        snprintf(history_path, sizeof(history_path), "./%s", HISTORY_FILE_NAME);
    }

    createFile(history_path);
    using_history();
    read_history(history_path);

    // prepare analyzing/ executing structures
    ExecuteEnvironment env;
    executorCtor(&env, env_table);

    Lexer lexer;
    Parser parser;
    lexerCtor(&lexer, NULL);
    parserCtor(&parser, &lexer);

    while(1) {
        // getting line
        char* line = readline("cyprSH> ");
        if(line == NULL) break;  // EOF (Ctrl+D)
        if(*line == '\0') {
            free(line);
            continue;
        }
        
        add_history(line);

        FILE* input = fmemopen(line, strlen(line), "r");
        if(input == NULL) {
            free(line);
            continue;
        }

        lexerReset(&lexer, input);
        parserReset(&parser);

        ASTNodePtr ast_root = ASTNodeCtor(NODE_PROGRAM, NULL, NULL);
        if(ast_root == NULL) {
            free(line);
            continue;
        }

        // analysis
        StatusEnum st = analyze(&parser, ast_root);
        if(st != SUCCESS) {
            free(line);
            ASTFreeTree(ast_root);
            continue;
        }

        // execution
        executeNode(ast_root, &env);

        ASTFreeTree(ast_root);
        free(line);
        if(env.flags & EXEC_FLAG_EXIT) {
            break;
        }
    }

    // cleanup
    write_history(history_path);
    parserDtor(&parser);
    lexerDtor(&lexer);
    return SUCCESS;
}


StatusEnum runScript(FILE* input, HashTablePtr env_table) {
    ExecuteEnvironment env;
    executorCtor(&env, env_table);

    Lexer lexer;
    Parser parser;
    lexerCtor(&lexer, input);
    parserCtor(&parser, &lexer);
    parserReset(&parser);

    ASTNodePtr ast_root = ASTNodeCtor(NODE_PROGRAM, NULL, NULL);
    if(ast_root == NULL) {
        parserDtor(&parser);
        lexerDtor(&lexer);
        return ERROR_MALLOC_FAILURE;
    }

    // analysis
    StatusEnum st = analyze(&parser, ast_root);
    if(st != SUCCESS) {
        parserDtor(&parser);
        lexerDtor(&lexer);
        ASTFreeTree(ast_root);
        return st;
    }

    // execution
    executeNode(ast_root, &env);

    ASTFreeTree(ast_root);
    parserDtor(&parser);
    lexerDtor(&lexer);
    return SUCCESS;
}


StatusEnum runString(char* string_input, HashTablePtr env_table) {
    FILE* input = fmemopen(string_input, strlen(string_input), "r");
    if(input == NULL) {
        printError("runString", "fmemopen failed");
        return ERROR_DEFAULT;
    }

    StatusEnum st = runScript(input, env_table);
    fclose(input);
    return st;
}