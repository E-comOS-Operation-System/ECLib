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
#ifndef ECLIB_SIGNAL_H
#define ECLIB_SIGNAL_H

#include <stdint.h>

// Signal numbers
#define ECLIB_SIGINT  2
#define ECLIB_SIGTERM 15
#define ECLIB_SIGCHLD 17
#define ECLIB_SIGKILL 9

typedef void (*eclib_sighandler_t)(int);
#define ECLIB_SIG_DFL ((eclib_sighandler_t)0)
#define ECLIB_SIG_IGN ((eclib_sighandler_t)1)

eclib_sighandler_t eclib_signal(int signum, eclib_sighandler_t handler);
int eclib_kill(int pid, int sig);

#endif
