/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 * 
 * All-in-one header for ECLib
 * This file is part of ECLib.
 */
#ifndef ECLIB_ALL_H
#define ECLIB_ALL_H

// Core headers
#include "error.h"
#include "start.h"
#include "service.h"
#include "ipc_message.h"

// File I/O
#include "file.h"

// Memory management
#include "men.h"

// Utilities
#include "utils.h"
#include "rui.h"

// POSIX-like extensions for porting
#include "process.h"
#include "filesystem.h"
#include "env.h"
#include "time.h"
#include "signal.h"
#include "pipe.h"

// E-comOS specific notes:
// - Path separator: '>' (not '/')
// - Pipe operator: '->' (not '|')

#endif
