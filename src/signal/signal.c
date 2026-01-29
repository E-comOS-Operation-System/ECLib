/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/signal.h"
#include "eclib/ipc_message.h"
#include "eclib/service.h"

#define SIGNAL_SERVICE_NAME "signal_service"
#define SIGNAL_CMD_REGISTER 0x5001
#define SIGNAL_CMD_KILL     0x5002

eclib_sighandler_t eclib_signal(int signum, eclib_sighandler_t handler) {
    uint32_t service_pid = eclib_service_lookup(SIGNAL_SERVICE_NAME);
    if (!service_pid) return ECLIB_SIG_DFL;
    
    struct {
        int signum;
        uint64_t handler_addr;
    } req = {signum, (uint64_t)handler};
    
    uint64_t old_handler = 0;
    size_t resp_len = sizeof(old_handler);
    
    if (ipc_call_sync(service_pid, SIGNAL_CMD_REGISTER, &req, sizeof(req),
                      &old_handler, &resp_len, 5000) != 0) {
        return ECLIB_SIG_DFL;
    }
    
    return (eclib_sighandler_t)old_handler;
}

int eclib_kill(int pid, int sig) {
    uint32_t service_pid = eclib_service_lookup(SIGNAL_SERVICE_NAME);
    if (!service_pid) return -1;
    
    struct {
        int pid;
        int sig;
    } req = {pid, sig};
    
    int result = -1;
    size_t resp_len = sizeof(result);
    
    if (ipc_call_sync(service_pid, SIGNAL_CMD_KILL, &req, sizeof(req),
                      &result, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return result;
}
