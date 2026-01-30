/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/time.h"
#include "eclib/ipc_message.h"
#include "eclib/service.h"

#define TIME_SERVICE_NAME "time_service"
#define TIME_CMD_GET 0x4001

eclib_time_t eclib_time(eclib_time_t* t) {
    uint32_t service_pid = eclib_service_lookup(TIME_SERVICE_NAME);
    if (!service_pid) return 0;
    
    int req = 0;
    eclib_time_t resp = 0;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, TIME_CMD_GET, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return 0;
    }
    
    if (t) *t = resp;
    return resp;
}
