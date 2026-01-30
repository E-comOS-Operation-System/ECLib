/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/pipe.h"
#include "eclib/ipc_message.h"
#include "eclib/service.h"

#define PIPE_SERVICE_NAME "pipe_service"
#define PIPE_CMD_CREATE 0x6001
#define PIPE_CMD_DUP2   0x6002

int eclib_pipe(int pipefd[2]) {
    uint32_t service_pid = eclib_service_lookup(PIPE_SERVICE_NAME);
    if (!service_pid) return -1;
    
    int req = 0;
    struct {
        int fd[2];
        int result;
    } resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, PIPE_CMD_CREATE, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    if (pipefd) {
        pipefd[0] = resp.fd[0];
        pipefd[1] = resp.fd[1];
    }
    
    return resp.result;
}

int eclib_dup2(int oldfd, int newfd) {
    uint32_t service_pid = eclib_service_lookup(PIPE_SERVICE_NAME);
    if (!service_pid) return -1;
    
    struct {
        int oldfd;
        int newfd;
    } req = {oldfd, newfd};
    
    int result = -1;
    size_t resp_len = sizeof(result);
    
    if (ipc_call_sync(service_pid, PIPE_CMD_DUP2, &req, sizeof(req),
                      &result, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return result;
}
