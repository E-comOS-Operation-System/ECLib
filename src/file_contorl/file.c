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

#include "../../include/eclib/file.h"
#include "../../include/eclib/error.h"
#include <stdint.h>
#include <stddef.h>
uint32_t eclib_service_lookup(const char* service_name);

char* eclib_strncpy(char* dest, const char* src, size_t n);
eclib_err_t ipc_call_sync(uint32_t pid, uint16_t msg_id, 
                         const void* req_data, size_t req_len,
                         void* resp_buf, size_t* resp_len,
                         uint32_t timeout_ms);


#undef ECLIB_FILE_INVALID
#define ECLIB_FILE_INVALID ((eclib_file_t)-1)

static uint32_t get_file_control_pid(void) {
    return eclib_service_lookup("file_control");  // From service_registry to get.
}
// -------------------------------
// Open file
// -------------------------------
eclib_file_t eclib_file_open(const char* filename, uint8_t mode) {
    if (filename == NULL || *filename == '\0') {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return ECLIB_FILE_INVALID;
    }
    uint32_t fc_pid = get_file_control_pid();
    if (fc_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);  // Cannot found service
        return ECLIB_FILE_INVALID;
    }
    eclib_file_open_req_t req;
    eclib_strncpy(req.filename, filename, sizeof(req.filename)-1);
    req.filename[sizeof(req.filename)-1] = '\0';  // Ensure null-termination
    req.mode = mode;
    // Send IPC message to file control service
    eclib_file_open_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        fc_pid,
        ECLIB_FILE_CMD_OPEN,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );
     if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return ECLIB_FILE_INVALID;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return ECLIB_FILE_INVALID;
    }

    // Return the opened file handle
    return resp.file;
}
// -------------------------------
// Read file
// -------------------------------
ssize_t eclib_file_read(eclib_file_t file, void* buf, size_t max_len) {
    // Check parameters
    if (file == ECLIB_FILE_INVALID || buf == NULL || max_len == 0) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return -1;
    }

    // Get file control service PID
    uint32_t fc_pid = get_file_control_pid();
    if (fc_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
        return -1;
    }

    // Build read request
    eclib_file_read_req_t req;
    req.file = file;
    req.max_len = max_len;

    // Send sync IPC message to file control service
    eclib_file_read_resp_t resp;
    resp.data = (uint8_t*)buf;  // Tell the service where to write data
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        fc_pid,
        ECLIB_FILE_CMD_READ,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );

    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return -1;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return -1;
    }

    // Success, return the actual read length
    return (ssize_t)resp.actual_len;
}
// -------------------------------
// Wirte file
// -------------------------------
ssize_t eclib_file_write(eclib_file_t file, const void* data, size_t len) {
    // Check parameters
    if (file == ECLIB_FILE_INVALID || data == NULL || len == 0) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return -1;
    }
    // Get PID
    uint32_t fc_pid = get_file_control_pid();
    if (fc_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
        return -1;
    }
    // Build write request
    eclib_file_write_req_t req;
    req.file = file;
    req.data = (const uint8_t*)data;
    req.data_len = len;
    // Send sync IPC message
     eclib_file_write_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        fc_pid,
        ECLIB_FILE_CMD_WRITE,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );

    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return -1;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return -1;
    }
    return (ssize_t)resp.actual_len; // Return actual written length
}
// -------------------------------
// Close file
// -------------------------------
eclib_err_t eclib_file_close(eclib_file_t file) {
    // Check parameters
    if (file == ECLIB_FILE_INVALID) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return ECLIB_ECLIB_INVALID_PARAMETER;
    }
    // Get PID
    uint32_t fc_pid = get_file_control_pid();
    if (fc_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
        return ECLIB_ECLIB_CANNOT_FIND_MODULE;
    }
    // Build close request
    eclib_file_close_req_t req;
    req.file = file;
    // Send sync IPC message
    eclib_file_close_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        fc_pid,
        ECLIB_FILE_CMD_CLOSE,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );
    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return err;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return resp.err;
    }
    return ECLIB_OK;
}
// -------------------------------
// Get file length
// -------------------------------
ssize_t eclib_file_get_length(eclib_file_t file, const char* filename) {
    // Check parameters
    if (file == ECLIB_FILE_INVALID && (filename == NULL || *filename == '\0')) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return -1;
    }

    // Get file_control service PID
    uint32_t fc_pid = get_file_control_pid();
    if (fc_pid == 0) {
        eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
        return -1;
    }

    // Build get length request
    eclib_file_get_len_req_t req;
    req.file = file;
    if (filename != NULL) {
        eclib_strncpy(req.filename, filename, sizeof(req.filename)-1);
        req.filename[sizeof(req.filename)-1] = '\0';
    } else {
        req.filename[0] = '\0';  // If filename is NULL, set to empty string
    }

    // Send sync IPC message
    eclib_file_get_len_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        fc_pid,
        ECLIB_FILE_CMD_GET_LEN,
        &req, sizeof(req),
        &resp, &resp_len,
        1000
    );

    if (err != ECLIB_OK) {
        eclib_set_last_err(err);
        return -1;
    }
    if (resp.err != ECLIB_OK) {
        eclib_set_last_err(resp.err);
        return -1;
    }

    // Success, return file length
    return (ssize_t)resp.len;
}
