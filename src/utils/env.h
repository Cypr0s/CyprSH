#ifndef ENV_H
#define ENV_H

#include "error.h"
#include "data_structures/htab.h"

StatusEnum populateEnvTable(HashTablePtr env_table, char** environ);

#endif // ENV_H