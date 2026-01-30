/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/filesystem.h"
#include "eclib/ipc_message.h"
#include "eclib/service.h"
#include "eclib/utils.h"

#define FS_SERVICE_NAME "file_service"
#define FS_CMD_STAT   0x3001
#define FS_CMD_ACCESS 0x3002
#define FS_CMD_UNLINK 0x3003
#define FS_CMD_CHDIR  0x3004
#define FS_CMD_GETCWD 0x3005

typedef struct {
    char path[256];
} fs_path_req_t;

typedef struct {
    eclib_stat_t stat;
    eclib_err_t err;
} stat_resp_t;

typedef struct {
    int result;
    eclib_err_t err;
} fs_resp_t;

typedef struct {
    char cwd[256];
    eclib_err_t err;
} getcwd_resp_t;

// Removed conflicting declaration of ipc_call_sync
// Ensure the correct declaration from ipc_message.h is used.

int eclib_stat(const char* path, eclib_stat_t* buf) {
    uint32_t service_pid = eclib_service_lookup(FS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    fs_path_req_t req = {0};
    eclib_strncpy(req.path, path, 255);
    
    stat_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, FS_CMD_STAT, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    if (buf) *buf = resp.stat;
    return resp.err == 0 ? 0 : -1;
}

int eclib_access(const char* path, int mode) {
    uint32_t service_pid = eclib_service_lookup(FS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    struct {
        char path[256];
        int mode;
    } req = {0};
    eclib_strncpy(req.path, path, 255);
    req.mode = mode;
    
    fs_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, FS_CMD_ACCESS, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return resp.result;
}

int eclib_unlink(const char* path) {
    uint32_t service_pid = eclib_service_lookup(FS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    fs_path_req_t req = {0};
    eclib_strncpy(req.path, path, 255);
    
    fs_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, FS_CMD_UNLINK, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return resp.result;
}

int eclib_chdir(const char* path) {
    uint32_t service_pid = eclib_service_lookup(FS_SERVICE_NAME);
    if (!service_pid) return -1;
    
    fs_path_req_t req = {0};
    eclib_strncpy(req.path, path, 255);
    
    fs_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, FS_CMD_CHDIR, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return -1;
    }
    
    return resp.result;
}

char* eclib_getcwd(char* buf, size_t size) {
    uint32_t service_pid = eclib_service_lookup(FS_SERVICE_NAME);
    if (!service_pid) return 0;
    
    int req = 0;
    getcwd_resp_t resp;
    size_t resp_len = sizeof(resp);
    
    if (ipc_call_sync(service_pid, FS_CMD_GETCWD, &req, sizeof(req),
                      &resp, &resp_len, 5000) != 0) {
        return 0;
    }
    
    if (buf && size > 0) {
        eclib_strncpy(buf, resp.cwd, size - 1);
        buf[size - 1] = '\0';
        return buf;
    }
    
    return 0;
}
