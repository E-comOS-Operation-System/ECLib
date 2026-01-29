/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 * 
 * This file is part of ECLib.
 * ECLib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */
#ifndef ECLIB_FILESYSTEM_H
#define ECLIB_FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "error.h"

// File access modes
#define ECLIB_F_OK 0
#define ECLIB_R_OK 4
#define ECLIB_W_OK 2
#define ECLIB_X_OK 1

// File stat structure
typedef struct {
    uint64_t mtime_sec;
    uint64_t mtime_nsec;
    uint64_t size;
    uint32_t mode;
} eclib_stat_t;

int eclib_stat(const char* path, eclib_stat_t* buf);
int eclib_access(const char* path, int mode);
int eclib_unlink(const char* path);
int eclib_chdir(const char* path);
char* eclib_getcwd(char* buf, size_t size);

#endif
