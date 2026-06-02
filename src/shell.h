#ifndef SHELL_H
#define SHELL_H

#include "utils/file.h"
#include "data_structures/htab.h"
#include "utils/env.h"
#include <readline/readline.h>
#include <readline/history.h>

#define HISTORY_FILE_PATH "./CyprSH_history"

StatusEnum runShell(int32_t file_descriptor, HashTablePtr env_table);

#endif // SHELL_H