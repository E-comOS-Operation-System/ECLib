/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/process.h"
#include "eclib/ipc_message.h"
#include "eclib/service.h"
#include "eclib/utils.h"

#define PROCESS_SERVICE_NAME "process_service"
#define PROCESS_CMD_FORK    0x2001
#define PROCESS_CMD_EXEC    0x2002
#define PROCESS_CMD_WAIT    0x2003
#define PROCESS_CMD_EXIT    0x2004
#define PROCESS_CMD_GETPPID 0x2005

typedef struct {
    int dummy;
} fork_req_t;

typedef struct {
    int pid;
    eclib_err_t err;
} fork_resp_t;

typedef struct {
    char path[256];
    char argv_data[1024];
    char envp_data[1024];
} exec_req_t;

typedef struct {
    eclib_err_t err;
} exec_resp_t;

typedef struct {
    int pid;
    int options;
} wait_req_t;

typedef struct {
    int pid;
    int status;
    eclib_err_t err;
} wait_resp_t;

int eclib_fork(void) {
    uint32_t service_pid = eclib_service_lookup(PROCESS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    fork_req_t req = {0};
    fork_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, PROCESS_CMD_FORK, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return resp.pid;
}

int eclib_execve(const char* path, char* const argv[], char* const envp[]) {
    uint32_t service_pid = eclib_service_lookup(PROCESS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    exec_req_t req = {0};
    eclib_strncpy(req.path, path, 255);
    
    // Serialize argv and envp (simplified)
    char* p = req.argv_data;
    for (int i = 0; argv && argv[i]; i++) {
        size_t len = eclib_strlen(argv[i]);
        if (p - req.argv_data + len + 1 < 1024) {
            eclib_memcpy(p, argv[i], len + 1);
            p += len + 1;
        }
    }
    
    p = req.envp_data;
    for (int i = 0; envp && envp[i]; i++) {
        size_t len = eclib_strlen(envp[i]);
        if (p - req.envp_data + len + 1 < 1024) {
            eclib_memcpy(p, envp[i], len + 1);
            p += len + 1;
        }
    }
    
    exec_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, PROCESS_CMD_EXEC, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return resp.err;
}

int eclib_wait(int* status) {
    return eclib_waitpid(-1, status, 0);
}

int eclib_waitpid(int pid, int* status, int options) {
    uint32_t service_pid = eclib_service_lookup(PROCESS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    wait_req_t req = {pid, options};
    wait_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, PROCESS_CMD_WAIT, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    if (status) *status = resp.status;
    return resp.pid;
}

void eclib_exit(int status) {
    uint32_t service_pid = eclib_service_lookup(PROCESS_SERVICE_NAME);
    if (service_pid) {
        ipc_send_msg(service_pid, 0, PROCESS_CMD_EXIT, sizeof(status), &status);
    }
    while(1);
}

uint32_t eclib_getppid(void) {
    uint32_t service_pid = eclib_service_lookup(PROCESS_SERVICE_NAME);
    if (!service_pid) return 0;
    
    int req = 0;
    uint32_t resp = 0;
    size_t resp_len = sizeof(resp);
    
    ipc_call_sync(service_pid, PROCESS_CMD_GETPPID, &req, sizeof(req),
                  &resp, &resp_len, 5000);
    
    return resp;
}
