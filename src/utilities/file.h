/**
 * @file        file.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   File operations (open, create)
 */

#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "error.h"

/** @brief Open file with given flags
 *  @param path File path
 *  @param flag Open flags (O_RDONLY, etc.)
 *  @param file_descriptor Output file descriptor
 *  @return Status code
 */
StatusEnum openFile(const char* path, uint32_t flag, int32_t* file_descriptor);

/** @brief Create new file (ignored if exists)
 *  @param name_path File path to create
 *  @return Status code
 */
StatusEnum createFile(const char* name_path);

#endif // FILE_H