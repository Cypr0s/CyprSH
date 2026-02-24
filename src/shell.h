#include "./utils/file.h"
#include "./data_structures/htab.h"
#include "./utils/env.h"
#include <readline/readline.h>
#include <readline/history.h>

#define HISTORY_FILE_PATH "./CyprSH_history"

StatusEnum run_shell(uint32_t file_descriptior, HashTablePtr env_table);