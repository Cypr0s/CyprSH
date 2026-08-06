/**
 * @file        env.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Environment variable table initialization
 */

#ifndef ENV_H
#define ENV_H

#include "error.h"
#include "data-structures/hash-table.h"

/** @brief Populate environment table from system environ
 *  @param env_table Hash table to populate
 *  @param environ System environment variables array
 *  @return Status code
 */
StatusEnum populateEnvTable(HashTablePtr env_table, char** environ);

#endif // ENV_H