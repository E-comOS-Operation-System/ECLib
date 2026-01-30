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

#ifndef ECLIB_IPC_MESSAGE_H
#define ECLIB_IPC_MESSAGE_H

#include "eclib/error.h" // Include centralized error type definition
#include <stdint.h>
#include <stddef.h>

// IPC message types
#define IPC_MSG_SHUTDOWN_REQUEST   0x53485554  // "SHUT"
#define IPC_MSG_EXIT_REQUEST       0x45584954  // "EXIT"
#define IPC_MSG_APPENDIX_S_WAIT    0x57414954  // "WAIT"
#define IPC_MSG_APPENDIX_S_OKTHANKS 0x4F4B5448 // "OKTH"
#define IPC_MSG_DO_NOT_KILL        0x444F4E54  // "DONT"

// IPC message structure
typedef struct ipc_message {
    uint32_t type;           // Message type
    uint32_t sender_pid;     // Sender PID
    uint32_t receiver_pid;   // Receiver PID
    uint32_t data_len;       // Data length
    uint8_t  data[256];      // Data
    uint64_t timestamp;      // Timestamp
    uint32_t flags;          // Flags
} ipc_message_t;

// IPC function prototypes
/*
 * Send a message to target process
 * Parameters:
 *   type: Message type
 *   flags: Flags for the message
 *   receiver_pid: Target process ID
 *   data_len: Size of payload (must be <= max IPC payload size)
 *   data: Pointer to message payload (will be copied to ipc_message_t.payload)
 * Return:
 *   ECLIB_OK: Sent successfully
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: No permission to send to target
 *   ECLIB_IPC_TIMEOUT: Timeout
 *   ECLIB_IPC_BUFFER_OVERFLOW: data_len exceeds max payload size
 */
int ipc_send_msg(uint32_t type, uint32_t flags, uint32_t receiver_pid, 
                 uint32_t data_len, const void* data);

/*
 * Receive a message
 * Parameters:
 *   msg: Pointer to ipc_message_t structure to receive the message
 *   timeout_ms: Timeout in milliseconds (0 = no timeout)
 * Return:
 *   ECLIB_OK: Message received
 *   ECLIB_IPC_TIMEOUT: Timeout
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 */
int ipc_receive_msg(ipc_message_t* msg, int timeout_ms);

/*
 * Broadcast a message to all processes
 * Parameters:
 *   type: Message type
 *   flags: Flags for the message
 *   data_len: Size of payload (must be <= max IPC payload size)
 *   data: Pointer to message payload (will be copied to ipc_message_t.payload)
 * Return:
 *   ECLIB_OK: Broadcasted successfully
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: No permission to send to target
 *   ECLIB_IPC_TIMEOUT: Timeout
 *   ECLIB_IPC_BUFFER_OVERFLOW: data_len exceeds max payload size
 */
int ipc_broadcast_msg(uint32_t type, uint32_t flags, uint32_t data_len, 
                     const void* data);

// Special IPC functions
/*
 * Protect background child processes from being reclaimed by kernel when main process exits
 * Description: After calling, all child processes of current main process (including future ones) will be marked as "independent processes".
 *              Kernel will not trigger SIGTERM/SIGKILL to child processes when main process exits (manual management required).
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: Insufficient permissions
 *   ECLIB_IPC_TIMEOUT: IPC service timeout
 */
int ipc_do_not_kill_sub(void);

/*
 * Emergency OK for do not kill sub
 * Description: In emergency situations, allow the main process to exit while keeping child processes alive.
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: Insufficient permissions
 *   ECLIB_IPC_TIMEOUT: IPC service timeout
 */
int ipc_do_not_kill_sub_emergency_ok(void);

/*
 * Request shutdown exemption
 * Parameters:
 *   timeout_ms: Timeout in milliseconds
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: Insufficient permissions
 *   ECLIB_IPC_TIMEOUT: IPC service timeout
 */
int ipc_request_shutdown_exemption(int timeout_ms);

/*
 * Perform a synchronous IPC call
 * Parameters:
 *   service_pid: Target service process ID
 *   cmd: Command to execute
 *   req: Pointer to request data
 *   req_len: Length of request data
 *   resp: Pointer to response buffer
 *   resp_len: Pointer to response buffer length
 *   timeout: Timeout in milliseconds
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_TIMEOUT: Timeout occurred
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not available
 */
eclib_err_t ipc_call_sync(uint32_t pid, uint16_t msg_id, const void* req_data, size_t req_len, void* resp_buf, size_t* resp_len, uint32_t timeout_ms);

/*
 * Receive an IPC message
 * Parameters:
 *   msg: Pointer to ipc_message_t structure to receive the message
 *   timeout_ms: Timeout in milliseconds
 * Return:
 *   ECLIB_OK: Message received successfully
 *   ECLIB_IPC_TIMEOUT: Timeout occurred
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not available
 */
int ipc_recv(ipc_message_t* msg, int timeout_ms);

/*
 * Get the current size of the IPC queue
 * Return:
 *   >0: Number of messages in the queue
 *   0: Queue is empty
 *   <0: Error code
 */
int ipc_get_queue_size(void);

/*
 * Peek the next message in the queue without removing it
 * Parameters:
 *   type: Output message type
 *   sender_pid: Output sender PID
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: Insufficient permissions
 *   ECLIB_IPC_TIMEOUT: IPC service timeout
 */
int ipc_peek_message(uint32_t* type, uint32_t* sender_pid);

/*
 * Clear the IPC queue
 * Return:
 *   ECLIB_OK: Success
 *   ECLIB_IPC_SERVICE_UNAVAIL: IPC service not running
 *   ECLIB_IPC_PERMISSION_DENIED: Insufficient permissions
 *   ECLIB_IPC_TIMEOUT: IPC service timeout
 */
int ipc_clear_queue(void);

#endif // ECLIB_IPC_MESSAGE_H