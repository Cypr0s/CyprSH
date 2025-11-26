#include "./utils/file.h"
#include "./data_structures/htab.h"
#include "./utils/env.h"

#define HISTORY_FILE_PATH "./CyprSH_history"

void run_shell(uint32_t file_descriptior, HashTablePtr env_table);