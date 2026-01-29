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
#ifndef ECLIB_SERVICE_H
#define ECLIB_SERVICE_H
#include <stdint.h>
#include <stddef.h>
#include "eclib/error.h" // Ensure eclib_err_t is included

// ---------------------
// The connted code with service registry
// ---------------------
#define SERVICE_CMD_LOOKUP    0x5001  // FOUND SERVICE
#define SERVICE_CMD_REGISTER  0x5002  // REGISTER SERVICE 
#define SERVICE_CMD_UNREGISTER 0x5003 // LOG OUT SERVICE
#define SERVICE_CMD_RESP      0x5004  
// ---------------------
// Communication structure (request/response format)
// ---------------------

typedef struct {
    char service_name[64]; // Service name
}service_lookup_req_t;

typedef struct {
    uint32_t service_pid; // Service PID (0 if not found)
    eclib_err_t err;      // Error code
} service_lookup_resp_t;

typedef struct {
    char service_name[64];  
    uint32_t pid;           
} service_register_req_t;

typedef struct {
    eclib_err_t err;        // ERROR CODE
} service_register_resp_t;

typedef struct {
    char service_name[64];
} service_unregister_req_t;

typedef struct {
    eclib_err_t err;        // ERROR CODE
} service_unregister_resp_t;
// ---------------------
// External Interface
// ---------------------
/*
* Found service's PID
* Parameter:
*    service_name: The name of the service to look up
* Return Value:
*    On success: service PID (non-zero)
*    On failure: 0 (and sets the last error code)
*/
uint32_t eclib_service_lookup(const char* service_name);

/*
* Register a service
* Parameter:
*    service_name: The name of the service to register
* Return Value:
*    On success: ECLIB_ERR_OK
*    On failure: The error code
*/
eclib_err_t eclib_service_register(const char* service_name);

/*
* Unregister a service
* Parameter:
*    service_name: The name of the service to unregister
* Return Value:
*    On success: ECLIB_ERR_OK
*    On failure: The error code
*/
eclib_err_t eclib_service_unregister(const char* service_name);
/*
* Get now service PID
* Return:Now service PID
*/
uint32_t eclib_getpid(void);

// 附录S关机保存信息
#define ServiceSaveInfo     "[Service] 正在保存状态"
#define ServiceStartInfo    "[Service] 服务启动"
#define ServiceStopInfo     "[Service] 服务停止"
#define ServiceErrorInfo    "[Service] 服务错误"

// 服务状态
enum service_state {
    SERVICE_RUNNING = 0,
    SERVICE_STOPPED,
    SERVICE_ERROR,
    SERVICE_SHUTDOWN
};

// 服务信息结构
struct service_info {
    int id;
    char name[64];
    enum service_state state;
    int pid;
    unsigned long long start_time;
    unsigned long long last_heartbeat;
};

// 服务注册/注销
int service_register(const char* name);
int service_unregister(int service_id);
int service_set_state(int service_id, enum service_state state);

// 服务心跳
int service_heartbeat(int service_id);

// 服务发现
int service_find_by_name(const char* name, struct service_info* info);
int service_list(struct service_info* list, int max_count);
#endif // ECLIB_SERVICE_H