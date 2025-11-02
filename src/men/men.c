/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 * 
 * This file is part of ECLib.
 * ECLib is free library; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */
#include "ipc_message.h"
#include "error.h"
#include "men.h"
#include "utils.h"
#include "service.h"
// Command codes agreed upon with the memory_manager service
#define MEM_CMD_MALLOC  0x2001
#define MEM_CMD_FREE    0x2002
#define MEM_CMD_REALLOC 0x2003

typedef struct {
    size_t size;
} mem_malloc_req_t;

typedef struct {
    void* addr;
    eclib_err_t err;
} mem_malloc_resp_t;

typedef struct {
    void* addr; 
} mem_free_req_t;

typedef struct {
    void* old_addr; 
    size_t new_size; 
} mem_realloc_req_t;

typedef struct {
    void* new_addr; 
    eclib_err_t err;
} mem_realloc_resp_t;
static uint32_t get_memory_manager_pid(void) {
    return eclib_service_lookup("memory_manager");
}
// Mentory Managaer
void* eclib_malloc(size_t size) {
    if (size == 0) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return NULL;
    }

    uint32_t mem_pid = get_memory_manager_pid();
    if (mem_pid == 0) {

        return NULL;
    }

  
    mem_malloc_req_t req = {.size = size};
    mem_malloc_resp_t resp;
    size_t resp_len = sizeof(resp);

    
    eclib_err_t err = ipc_call_sync(
        mem_pid, MEM_CMD_MALLOC,
        &req, sizeof(req),
        &resp, &resp_len,
        1000  // timeout 1s
    );

    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return NULL;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return NULL;
    }

    return resp.addr;
}

void eclib_free(void* addr) {
    if (addr == NULL) return;

    uint32_t mem_pid = get_memory_manager_pid();
    if (mem_pid == 0) {
        return;
    }


    mem_free_req_t req = {.addr = addr};

 
    ipc_call_sync(
        mem_pid, MEM_CMD_FREE,
        &req, sizeof(req),
        NULL, NULL,
        500  
    );
}

void* eclib_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return NULL;
    }
    size_t total_size = nmemb * size;

    if (size != 0 && total_size / size != nmemb) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_ALLOCATE_MEMORY);
        return NULL;
    }

    void* ptr = eclib_malloc(total_size);
    if (ptr != NULL) {
        eclib_memset(ptr, 0, total_size);
    }
    return ptr;
}

void* eclib_realloc(void* ptr, size_t size) {
    if (size == 0) {
        eclib_free(ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return eclib_malloc(size);
    }

    uint32_t mem_pid = get_memory_manager_pid();
    if (mem_pid == 0) {
        return NULL;
    }


    mem_realloc_req_t req = {
        .old_addr = ptr,
        .new_size = size
    };
    mem_realloc_resp_t resp;
    size_t resp_len = sizeof(resp);


    eclib_err_t err = ipc_call_sync(
        mem_pid, MEM_CMD_REALLOC,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );

    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return NULL;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return NULL;
    }

    return resp.new_addr;
}