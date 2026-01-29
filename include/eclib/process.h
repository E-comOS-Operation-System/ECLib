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
#ifndef ECLIB_PROCESS_H
#define ECLIB_PROCESS_H

#include <stdint.h>
#include "error.h"

// Wait options
#define ECLIB_WNOHANG 1

// Exit status macros
#define ECLIB_WIFEXITED(status) (((status) & 0xFF) == 0)
#define ECLIB_WEXITSTATUS(status) (((status) >> 8) & 0xFF)

int eclib_fork(void);
int eclib_execve(const char* path, char* const argv[], char* const envp[]);
int eclib_wait(int* status);
int eclib_waitpid(int pid, int* status, int options);
void eclib_exit(int status) __attribute__((noreturn));
uint32_t eclib_getppid(void);

#endif
