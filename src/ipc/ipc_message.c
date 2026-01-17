/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 * 
 * This file is part of ECLib.
 * ECLib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */
#include "../../include/eclib/ipc_message.h"
#include "../../include/eclib/error.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
uint32_t sys_get_current_pid(void);
int sys_ipc_send_message(ipc_message_t *msg, int need_feedback, uint64_t seq);
uint32_t sys_get_tick(void);
eclib_err_t ipc_get_msg_state(uint64_t seq, uint8_t *state);


// Custom implementations
extern void* eclib_malloc(size_t size);
extern void eclib_free(void* ptr);
extern void* eclib_memset(void* ptr, int value, size_t num);
extern uint32_t eclib_getpid(void);
extern void eclib_usleep(uint32_t usec);

typedef struct {
    volatile int locked;
} eclib_mutex_t;

static void eclib_mutex_init(eclib_mutex_t* mutex) {
    mutex->locked = 0;
}

static void eclib_mutex_lock(eclib_mutex_t* mutex) {
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        while (mutex->locked) {}
    }
}

static void eclib_mutex_unlock(eclib_mutex_t* mutex) {
    __sync_lock_release(&mutex->locked);
}
// Define maximum IPC payload size
#define IPC_MAX_RECV_QUEUE 16
#define IPC_MAX_SENT_RECORDS 32
#define IPC_MAX_PAYLOAD_LEN 4096

// IPC message states and structures
typedef enum {
    ECILB_IPC_STATE_PENDING = 0,
    ECLIB_IPC_STATE_SENT = 1,
    ECLIB_IPC_STATE_SENDING = 2,
    ECLIB_IPC_STATE_SENT_FAILED = 3,
    ECLIB_IPC_STATE_DELIVERED = 4,
    ECLIB_IPC_STATE_DELIVERY_FAILED = 5,
    ECLIB_IPC_STATE_RECEIVED = 6,
    ECLIB_IPC_STATE_PD = 7, // Permission denied
    ECLIB_IPC_STATE_LOST_MESSAGE = 8,
    ECLIB_IPC_STATE_LOADING = 9,
    ECLIB_IPC_STATE_UNLOADING = 10,
    ECLIB_IPC_STATE_UNLOADED = 11,
    ECLIB_IPC_STATE_WATING_FOR_KERNEL = 12,
    ECLIB_IPC_STATE_OTHER = 13,
    ECLIB_IPC_STATE_UNKNOWN = 14
} eclib_ipc_msg_state_e;

// IPC meassage sent states log
typedef struct{
    uint64_t msg_seq;
    eclib_ipc_msg_state_e state;
    uint32_t dest_pid;
    uint16_t msg_id;
    uint32_t timeout_ms;
    uint16_t send_time;
} ipc_sent_msg_record_t;

// IPC global state of the current process
typedef struct{
    int inited;
    ipc_message_t* recv_queue[IPC_MAX_RECV_QUEUE]; // Receive queue
    size_t recv_queue_len;
    ipc_sent_msg_record_t sent_records[IPC_MAX_SENT_RECORDS];
    size_t sent_records_len;
    uint64_t next_seq;
    void* lock; // During development, the E-comOS User Mode Team Library Group did not communicate effectively with the E-comOS Kernel Team. Therefore, the functions here may not be the actual functions implemented in the kernel.
} ipc_process_state_t;

// Global IPC state
static ipc_process_state_t g_ipc_state = {0};

static eclib_err_t ipc_init(void) {
    // Initialize IPC state
    if (g_ipc_state.inited) {
        return ECLIB_OK;
    }

    g_ipc_state.lock = eclib_malloc(sizeof(eclib_mutex_t));
    if (g_ipc_state.lock == NULL) {
        return ECLIB_ECLIB_CANNOT_ALLOCATE_MEMORY;
    }
    eclib_mutex_init((eclib_mutex_t*)g_ipc_state.lock);

    eclib_memset(g_ipc_state.recv_queue, 0, sizeof(g_ipc_state.recv_queue));
    g_ipc_state.recv_queue_len = 0;
    eclib_memset(g_ipc_state.sent_records, 0, sizeof(g_ipc_state.sent_records));
    g_ipc_state.sent_records_len = 0;

    g_ipc_state.next_seq = 1; // Start from 1
    g_ipc_state.inited = 1;
    return ECLIB_OK;
}

static eclib_err_t ipc_check_init(void) {
    if (!g_ipc_state.inited) {
        return ipc_init();
    }
    return ECLIB_OK;
}

eclib_err_t ipc_do_not_kill_sub(void) {
    // Placeholder implementation
     eclib_err_t ret = ipc_check_init();
    if (ret != ECLIB_OK) {
        return ret;
    }
    // Construct the message content for "child process protection" (telling the kernel to "protect the child processes of the current process")
    // Custom message format: for example, storing the current process's PID in the payload (the kernel needs to know which process's child processes to protect)
    uint32_t current_pid = eclib_getpid();
    size_t payload_len = sizeof(current_pid);
    uint64_t msg_seq;
    ret = ipc_send_msg(
        0,
        0x1001, // Custom message ID for "do not kill sub processes"
        &current_pid,
        payload_len,
        1,
        &msg_seq
    );
    if (ret != ECLIB_OK) {
        return ret;
    }
    eclib_ipc_msg_state_e state;
    uint32_t wait_count = 0;
    while (1) { // We vaguely remember that the kernel PID was 0.
        ret = ipc_get_msg_state(msg_seq, (uint8_t*)&state);
        if (ret != ECLIB_OK) {
            return ret;
        }
        if (state == ECLIB_IPC_STATE_SENT || state == ECLIB_IPC_STATE_DELIVERED){
            break;
        }
        if (state == ECLIB_IPC_STATE_SENT_FAILED || state == ECLIB_IPC_STATE_DELIVERY_FAILED) {
            return ECLIB_IPC_STATE_SENT_FAILED;
        }
        if (wait_count >= 10){
            return ECLIB_IPC_TIMEOUT;
        }
        eclib_usleep(10000);
        wait_count++;
    }

    return ECLIB_OK;
}
#define MAX_VALID_PID 0xFFFF
eclib_err_t ipc_send_msg(uint32_t pid, uint16_t msg_id, const void* data, size_t data_len,
                        int need_feedback, uint64_t* msg_seq) {
    eclib_err_t ret = ipc_check_init();
    if (ret != ECLIB_OK) {
        return ret;
    }
    if (pid > MAX_VALID_PID) {
        return ECLIB_IPC_PERMISSION_DENIED;
    }
    if (data_len > IPC_MAX_PAYLOAD_LEN) {
        return ECLIB_IPC_BUFFER_OVERFLOW;
    }
    eclib_mutex_lock((eclib_mutex_t*)g_ipc_state.lock);
    uint64_t seq = g_ipc_state.next_seq++;
    if (msg_seq != NULL) {
        *msg_seq = seq;
    }
    ipc_message_t* msg = (ipc_message_t*)eclib_malloc(sizeof(ipc_message_t) + data_len);
    if (msg == NULL) {
        eclib_mutex_unlock((eclib_mutex_t*)g_ipc_state.lock);
        return ECLIB_ECLIB_CANNOT_ALLOCATE_MEMORY;
    }
    msg->message_id = msg_id;
    msg->sender_pid = sys_get_current_pid();
    msg->receiver_pid = pid;
    msg->payload_size = data_len;
    if (data != NULL && data_len > 0) {
        memcpy(msg->payload, data, data_len);
    }
    ret = sys_ipc_send_message(msg, need_feedback, seq);
    if (g_ipc_state.sent_records_len < IPC_MAX_SENT_RECORDS) {
        ipc_sent_msg_record_t* record = &g_ipc_state.sent_records[g_ipc_state.sent_records_len++];
        record->msg_seq = seq;
        record->dest_pid = pid;
        record->msg_id = msg_id;
        record->send_time = sys_get_tick();
        record->timeout_ms = 1000;
        record->state = (ret == 0) ? ECLIB_IPC_STATE_SENT : ECLIB_IPC_STATE_SENT_FAILED;
    } else {
        ret = ECLIB_ECLIB_RESOURCE_LIMIT;
    }
    eclib_mutex_unlock((eclib_mutex_t*)g_ipc_state.lock);
    eclib_free(msg);
    if (ret == 0) return ECLIB_OK;
    else if (ret == -1) return ECLIB_IPC_INVALID_ENDPOINT;
    else if (ret == -2) return ECLIB_IPC_MSG_QUEUE_FULL;
    else return ECLIB_IPC_SERVICE_UNAVAIL;
}
static void ipc_gen_send_failed_msg(uint64_t seq, uint32_t dest_pid, uint16_t msg_id, int sys_err) {
    typedef struct {
        uint64_t original_seq;    
        uint32_t original_dest;
        uint16_t original_msg_id; // Message ID
        eclib_err_t error_code;   // ERROR CODE
    } ipc_send_failed_payload_t;
    size_t payload_len = sizeof(ipc_send_failed_payload_t);
    ipc_message_t* fail_msg = eclib_malloc(sizeof(ipc_message_t) + payload_len);
    if (fail_msg == NULL) {
        return;
    }
    fail_msg->message_id = ECLIB_IPC_MSG_TYPE_SEND_FAILED;
    fail_msg->sender_pid = 0;
    fail_msg->receiver_pid =eclib_getpid();
    fail_msg->payload_size = payload_len;

    ipc_send_failed_payload_t* payload = (ipc_send_failed_payload_t*)fail_msg->payload;
    payload->original_seq = seq;
    payload->original_dest = dest_pid;
    payload->original_msg_id = msg_id;

    if (sys_err == -1) {
        payload->error_code = ECLIB_IPC_INVALID_ENDPOINT;
    } else if (sys_err == -2) {
        payload->error_code = ECLIB_IPC_MSG_QUEUE_FULL;
    } else {
        payload->error_code = ECLIB_IPC_SERVICE_UNAVAIL;
    }
    eclib_mutex_lock((eclib_mutex_t*)g_ipc_state.lock);
    if (g_ipc_state.recv_queue_len < IPC_MAX_RECV_QUEUE) {
        g_ipc_state.recv_queue[g_ipc_state.recv_queue_len++] = fail_msg;
    } else {
        eclib_free(fail_msg);
    }
    eclib_mutex_unlock((eclib_mutex_t*)g_ipc_state.lock);
    
}
