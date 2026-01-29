#include "eclib/ipc_message.h" // Updated include directive
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <stdio.h>


#define SYS_IPC_SEND          1001
#define SYS_IPC_RECEIVE       1002
#define SYS_IPC_DO_NOT_KILL   1003
#define SYS_IPC_BROADCAST     1004

int ipc_send_msg(uint32_t type, uint32_t flags, uint32_t receiver_pid, 
                 uint32_t data_len, const void* data) {
    struct ipc_message msg = {0};
    
    msg.type = type;
    msg.sender_pid = getpid();
    msg.receiver_pid = receiver_pid;
    msg.data_len = (data_len > 256) ? 256 : data_len;
    msg.flags = flags;
    msg.timestamp = time(NULL);
    
    if (data && data_len > 0) {
        memcpy(msg.data, data, msg.data_len);
    }
    
    // 系统调用发送
    return syscall(SYS_IPC_SEND, &msg);
}

int ipc_receive_msg(struct ipc_message* msg, int timeout_ms) {
    if (!msg) {
        return -1;
    }
    
    // 系统调用接收
    int ret = syscall(SYS_IPC_RECEIVE, msg, timeout_ms);
    
    if (ret >= 0 && msg->timestamp == 0) {
        msg->timestamp = time(NULL);
    }
    
    return ret;
}

int ipc_broadcast_msg(uint32_t type, uint32_t flags, uint32_t data_len, 
                     const void* data) {
    struct ipc_message msg = {0};
    
    msg.type = type;
    msg.sender_pid = getpid();
    msg.receiver_pid = 0xFFFFFFFF;  // 广播地址
    msg.data_len = (data_len > 256) ? 256 : data_len;
    msg.flags = flags;
    msg.timestamp = time(NULL);
    
    if (data && data_len > 0) {
        memcpy(msg.data, data, msg.data_len);
    }
    
    return syscall(SYS_IPC_BROADCAST, &msg);
}

int ipc_do_not_kill_sub(void) {
    // 系统调用设置标志
    int ret = syscall(SYS_IPC_DO_NOT_KILL, 0);
    
    if (ret == 0) {
        // 发送IPC消息通知PowerOffer
        char data[32];
        snprintf(data, sizeof(data), "DO_NOT_KILL:%d", getpid());
        ipc_broadcast_msg(IPC_MSG_DO_NOT_KILL, 0, strlen(data), data);
    }
    
    return ret;
}

int ipc_do_not_kill_sub_emergency_ok(void) {
    // 系统调用设置标志（紧急情况下可杀）
    int ret = syscall(SYS_IPC_DO_NOT_KILL, 1);
    
    if (ret == 0) {
        char data[32];
        snprintf(data, sizeof(data), "DO_NOT_KILL_EMERGENCY:%d", getpid());
        ipc_broadcast_msg(IPC_MSG_DO_NOT_KILL, 0, strlen(data), data);
    }
    
    return ret;
}