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
#include "eclib/service.h"
#include "eclib/ipc_message.h" // Updated include directive
#include "eclib/error.h"
#include "eclib/utils.h"
#include <stdint.h>
static uint32_t get_registry_pid(void) {
    return 1;
}
// ---------------------
// Found service's PID
// ---------------------
uint32_t eclib_service_lookup(const char* service_name) {
    if (service_name == NULL || *service_name == '\0') {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return 0;
    }
    uint32_t registry_pid = get_registry_pid();
    if (registry_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
        return 0;
    }
    service_lookup_req_t req;
    eclib_strncpy(req.service_name, service_name, sizeof(req.service_name)-1);
    req.service_name[sizeof(req.service_name)-1] = '\0';
    service_lookup_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        registry_pid,
        SERVICE_CMD_LOOKUP,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );
    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return 0;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return 0;
    }
    return resp.service_pid;
}
// ---------------------
// Register service
// ---------------------
eclib_err_t eclib_service_register(const char* service_name) {
    if (service_name == NULL || *service_name == '\0') {
        return eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
    }
    uint32_t registry_pid = get_registry_pid();
    if (registry_pid == 0) {
        return eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
    }
    service_register_req_t req;
    eclib_strncpy(req.service_name, service_name, sizeof(req.service_name)-1);
    req.service_name[sizeof(req.service_name)-1] = '\0';
    req.pid = eclib_getpid();
    service_register_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        registry_pid,
        SERVICE_CMD_REGISTER,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );
    if (err != ECLIB_OK) {
        return eclib_set_last_err(err);
    }
    if (resp.err != ECLIB_OK) {
        return eclib_set_last_err(resp.err);
    }
    return ECLIB_OK;
}
// ---------------------
// Unregister service
// ---------------------
eclib_err_t eclib_service_unregister(const char* service_name) {
    if (service_name == NULL || *service_name == '\0') {
        return eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
    }
    uint32_t registry_pid = get_registry_pid();
    if (registry_pid == 0) {
        return eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
    }
    service_unregister_req_t req;
    eclib_strncpy(req.service_name, service_name, sizeof(req.service_name)-1);
    req.service_name[sizeof(req.service_name)-1] = '\0';
    service_unregister_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        registry_pid,
        SERVICE_CMD_UNREGISTER,
        &req, sizeof(req),
        &resp, &resp_len,
        500
    );
    if (err!=ECLIB_OK){
        return eclib_set_last_err(err);
    }
    return resp.err;
}

// -------------------------------
// Get now service PID
// -------------------------------
uint32_t eclib_getpid(void) {
    extern uint32_t sys_getpid(void);  // Kernel system call to get current PID
    return sys_getpid();
}
