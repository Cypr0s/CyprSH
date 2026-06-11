#include "shell.h"



int32_t main(int32_t argc, char **argv) {
    extern char **environ;
    int32_t file_descriptor = STDIN_FILENO;
    if(argc == 2) {
        openFile(argv[1], O_RDONLY, &file_descriptor);
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

// will need revorking AI generated slop
StatusEnum runShell(int32_t file_descriptor, HashTablePtr env_table) {
    if(!isatty(file_descriptor)) {
        // TODO: script mode
        return SUCCESS;
    }

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

    // build the execution environment once — reused for every command
    ExecuteEnvironment env = {
        .env_table = env_table,
        .flags = EXEC_FLAG_NONE,
        .last_exec_status = 0,
    };

    // lexer and parser created once, reused via reset
    Lexer lexer;
    Parser parser;
    lexerCtor(&lexer, NULL);
    parserCtor(&parser, &lexer);

    while(1) {
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

        StatusEnum st = analyze(&parser, ast_root);
        if(st == SUCCESS) {
            #ifdef DEBUG
            printAST(ast_root, 0);
            #endif
            executeNode(ast_root, &env);
        }

        ASTFreeTree(ast_root);
        free(line);
        if(env.flags & EXEC_FLAG_EXIT) {
            break;
        }
    }


    write_history(history_path);

    parserDtor(&parser);
    lexerDtor(&lexer);
    return SUCCESS;
}


// ai slop
void printAST(ASTNodePtr node, int depth) {
    if(!node) return;

    // indent
    for(int i = 0; i < depth; i++) fprintf(stderr, "  ");

    // node type name
    const char* type_names[] = {
        [NODE_PROGRAM]          = "PROGRAM",
        [NODE_COMPLETE_COMMAND] = "COMPLETE_COMMAND",
        [NODE_LIST]             = "LIST",
        [NODE_AND_OR]           = "AND_OR",
        [NODE_PIPELINE]         = "PIPELINE",
        [NODE_SIMPLE_COMMAND]   = "SIMPLE_COMMAND",
        [NODE_CMD_PREFIX]       = "CMD_PREFIX",
        [NODE_CMD_WORD]         = "CMD_WORD",
        [NODE_CMD_SUFFIX]       = "CMD_SUFFIX",
        [NODE_REDIRECT]         = "REDIRECT",
        [NODE_ASSIGNMENT_WORD]  = "ASSIGNMENT_WORD",
        [NODE_WORD]             = "WORD",
        [NODE_IO_NUM]           = "IO_NUM",
        [NODE_SUBSHELL]         = "SUBSHELL",
        [NODE_BRACE_GROUP]      = "BRACE_GROUP",
        [NODE_IF_CLAUSE]        = "IF_CLAUSE",
        [NODE_ELSE_CLAUSE]      = "ELSE_CLAUSE",
        [NODE_WHILE_CLAUSE]     = "WHILE_CLAUSE",
        [NODE_UNTIL_CLAUSE]     = "UNTIL_CLAUSE",
        [NODE_FOR_CLAUSE]       = "FOR_CLAUSE",
        [NODE_CASE_CLAUSE]      = "CASE_CLAUSE",
        [NODE_CASE_ITEM]        = "CASE_ITEM",
        [NODE_FUNCTION_DEF]     = "FUNCTION_DEF"
    };

    fprintf(stderr, "[%s]", type_names[node->type]);

    // value if present
    if(node->value) fprintf(stderr, " value='%s'", node->value);

    // flags if set
    if(node->flags) fprintf(stderr, " flags=%d", node->flags);

    fprintf(stderr, "\n");

    // recurse into children
    for(int i = 0; i < node->num_children; i++) {
        printAST(node->children[i], depth + 1);
    }
}