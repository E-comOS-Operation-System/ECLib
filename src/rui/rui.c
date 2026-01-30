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
#include "eclib/rui.h" // Corrected include path
#include "eclib/service.h" // Added for eclib_service_lookup
#include "eclib/utils.h" // Added for eclib_strncpy
#include "eclib/ipc_message.h"
#include <string.h> // Added for memcpy

// Global variable: RUI service PID (valid after initialization)
static uint32_t g_rui_pid = 0;

// 1. Initialize RUI communication (retrieve RUI service PID)
eclib_err_t eclib_rui_init(void) {
    if (g_rui_pid != 0) return ECLIB_OK; // Already initialized

    g_rui_pid = eclib_service_lookup("rui_service");
    if (g_rui_pid == 0) {
        return eclib_set_last_err(ECLIB_ECLIB_CANNOT_FIND_MODULE);
    }
    return ECLIB_OK;
}

// 2. Create a window
uint32_t eclib_rui_create_window(const rui_point_t* pos, const rui_size_t* size, 
                                const char* title, const rui_color_t* bg_color) {
    // Check initialization and parameters
    if (g_rui_pid == 0 && eclib_rui_init() != ECLIB_OK) return 0;
    if (!pos || !size || !title || !bg_color) {
        eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
        return 0;
    }

    // Construct request
    rui_window_create_req_t req;
    req.pos = *pos;
    req.size = *size;
    eclib_strncpy(req.title, title, sizeof(req.title)-1);
    req.title[sizeof(req.title)-1] = '\0';
    req.bg_color = *bg_color;

    // Synchronous call to RUI service
    rui_window_create_resp_t resp;
    size_t resp_len = sizeof(resp);
    eclib_err_t err = ipc_call_sync(
        g_rui_pid, RUI_CMD_WINDOW_CREATE,
        &req, sizeof(req),
        &resp, &resp_len,
        500
    );

    if (err != ECLIB_OK || resp.err != ECLIB_OK) {
        eclib_set_last_err(err);
        return 0;
    }
    return resp.window_id; // Return window ID
}

// 3. Draw text (other functions are similar, repetitive logic omitted)
eclib_err_t eclib_rui_draw_text(uint32_t window_id, const rui_point_t* pos, 
                               const char* text, const rui_color_t* color, uint8_t font_size) {
    if (g_rui_pid == 0 && eclib_rui_init() != ECLIB_OK) return ECLIB_ECLIB_CANNOT_FIND_MODULE;
    if (!pos || !text || !color || window_id == 0) {
        return eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);
    }

    rui_draw_text_req_t req;
    req.window_id = window_id;
    req.pos = *pos;
    eclib_strncpy(req.text, text, sizeof(req.text)-1);
    req.text[sizeof(req.text)-1] = '\0';
    req.color = *color;
    req.font_size = font_size;

    // Drawing text does not require return data, only confirmation of success
    return ipc_call_sync(
        g_rui_pid, RUI_CMD_DRAW_TEXT,
        &req, sizeof(req),
        NULL, NULL,
        500
    );
}

// 4. Wait for events (blocking to receive asynchronous events sent by RUI)
eclib_err_t eclib_rui_wait_event(rui_event_t* event, uint32_t timeout_ms) {
    if (g_rui_pid == 0 && eclib_rui_init() != ECLIB_OK) return ECLIB_ECLIB_CANNOT_FIND_MODULE;
    if (!event) return eclib_set_last_err(ECLIB_ECLIB_INVALID_PARAMETER);

    // Receive "event notification" type messages sent by RUI (custom message type needs to be defined in IPC)
    #define ECLIB_IPC_MSG_TYPE_RUI_EVENT 0x2001

    ipc_message_t msg = {0};
    msg.type = ECLIB_IPC_MSG_TYPE_RUI_EVENT;

    eclib_err_t err = ipc_recv(&msg, timeout_ms);
    if (err == ECLIB_OK && msg.data_len == sizeof(rui_event_t)) {
        memcpy(event, msg.data, sizeof(rui_event_t));
    } else {
        err = ECLIB_IPC_BUFFER_OVERFLOW;
    }

    return err;
}