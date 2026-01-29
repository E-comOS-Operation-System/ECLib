#include "eclib/shutdown.h"
#include "eclib/ipc_message.h"
#include "eclib/capability.h"
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h> // Added for snprintf

#define SYS_SHUTDOWN          2001
#define SYS_REBOOT            2002
#define SYS_REGISTER_CALLBACK 2003


static struct shutdown_callback {
    shutdown_callback_t callback;
    void* user_data;
    int registered;
} g_callbacks[32];

static int g_callback_count = 0;

int shutdown_system(enum shutdown_reason reason, uint32_t flags, 
                    const char* message) {
    // 检查关机权限
    if (!check_capability(CAPABILITY_SHUTDOWN)) {
        return -1;  // 权限不足
    }
    
    // 构建关机消息
    char msg_buf[256];
    if (message && strlen(message) > 0) {
        snprintf(msg_buf, sizeof(msg_buf), "%s", message);
    } else {
        snprintf(msg_buf, sizeof(msg_buf), "用户请求关机");
    }
    
    // 发送IPC广播通知所有进程
    if (!(flags & SHUTDOWN_FLAG_NOWAIT)) {
        char ipc_msg[512];
        snprintf(ipc_msg, sizeof(ipc_msg), 
                 "SHUTDOWN:reason=%d:message=%s", reason, msg_buf);
        ipc_broadcast_msg(IPC_MSG_SHUTDOWN_REQUEST, flags, 
                         strlen(ipc_msg), ipc_msg);
    }
    
    // 执行关机
    return syscall(SYS_SHUTDOWN, reason, flags, msg_buf);
}

int reboot_system(enum shutdown_reason reason, uint32_t flags,
                  const char* message) {
    if (!check_capability(CAPABILITY_REBOOT)) {
        return -1;
    }
    
    char msg_buf[256];
    if (message && strlen(message) > 0) {
        snprintf(msg_buf, sizeof(msg_buf), "%s", message);
    } else {
        snprintf(msg_buf, sizeof(msg_buf), "用户请求重启");
    }
    
    if (!(flags & SHUTDOWN_FLAG_NOWAIT)) {
        char ipc_msg[512];
        snprintf(ipc_msg, sizeof(ipc_msg), 
                 "REBOOT:reason=%d:message=%s", reason, msg_buf);
        ipc_broadcast_msg(IPC_MSG_SHUTDOWN_REQUEST, flags, 
                         strlen(ipc_msg), ipc_msg);
    }
    
    return syscall(SYS_REBOOT, reason, flags, msg_buf);
}

int register_shutdown_callback(shutdown_callback_t callback, void* user_data) {
    if (!callback || g_callback_count >= 32) {
        return -1;
    }
    
    // 检查是否已注册
    for (int i = 0; i < g_callback_count; i++) {
        if (g_callbacks[i].callback == callback) {
            return -2;  // 已注册
        }
    }
    
    // 注册新回调
    g_callbacks[g_callback_count].callback = callback;
    g_callbacks[g_callback_count].user_data = user_data;
    g_callbacks[g_callback_count].registered = 1;
    g_callback_count++;
    
    return 0;
}

int appendix_s_begin_save(void) {
    // 开始附录S保存协议
    ipc_do_not_kill_sub();
    
    // 获取PowerOffer的PID（假设是1000）
    int poweroffer_pid = 1000;
    
    // 发送WAIT消息
    char wait_msg[5] = "WAIT";
    return ipc_send_msg(IPC_MSG_APPENDIX_S_WAIT, 0, poweroffer_pid, 4, wait_msg);
}