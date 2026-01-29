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
#ifndef ECLIB_TIME_H
#define ECLIB_TIME_H

#include <stdint.h>

typedef uint64_t eclib_time_t;

eclib_time_t eclib_time(eclib_time_t* t);

#endif
