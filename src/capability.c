#include "eclib/capability.h"
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>

// 系统调用号
#define SYS_ACQUIRE_CAPABILITY  3001
#define SYS_RELEASE_CAPABILITY  3002
#define SYS_VALIDATE_CAPABILITY 3003
#define SYS_SEND_VIRTUAL_PORT   3004
#define SYS_RECEIVE_VIRTUAL_PORT 3005

struct capability_token* acquire_capability(uint32_t capabilities, 
                                           const char* description,
                                           uint32_t timeout_sec) {
    if (!description) {
        return NULL;
    }
    
    // 准备参数
    char desc_buf[65];
    strncpy(desc_buf, description, 64);
    desc_buf[64] = '\0';
    
    // 系统调用获取令牌
    struct capability_token* token = (struct capability_token*)
        syscall(SYS_ACQUIRE_CAPABILITY, capabilities, desc_buf, timeout_sec);
    
    if (token) {
        token->owner_pid = getpid();
    }
    
    return token;
}

int release_capability(struct capability_token* token) {
    if (!token) {
        return -1;
    }
    
    // 检查所有权
    if (token->owner_pid != getpid()) {
        return -2;  // 不是拥有者
    }
    
    int ret = syscall(SYS_RELEASE_CAPABILITY, token);
    
    if (ret == 0) {
        // 清空令牌
        memset(token, 0, sizeof(struct capability_token));
    }
    
    return ret;
}

int send_token_to_virtual_port(struct capability_token* token) {
    if (!token) {
        return -1;
    }
    
    // 检查所有权
    if (token->owner_pid != getpid()) {
        return -2;
    }
    
    // 检查有效期
    uint32_t now = time(NULL);
    if (now > token->valid_until) {
        return -3;  // 令牌已过期
    }
    
    return syscall(SYS_SEND_VIRTUAL_PORT, token);
}