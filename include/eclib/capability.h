#ifndef ECLIB_CAPABILITY_H
#define ECLIB_CAPABILITY_H

#include <stdint.h>

// 能力位
#define CAPABILITY_SHUTDOWN      (1 << 0)  // 关机能力
#define CAPABILITY_REBOOT        (1 << 1)  // 重启能力
#define CAPABILITY_SUSPEND       (1 << 2)  // 挂起能力
#define CAPABILITY_HIBERNATE     (1 << 3)  // 休眠能力
#define CAPABILITY_KILL_PROCESS  (1 << 4)  // 杀死进程
#define CAPABILITY_CHANGE_CONFIG (1 << 5)  // 修改配置
#define CAPABILITY_ALL           (0xFFFFFFFF)  // 所有能力

// 能力令牌结构
struct capability_token {
    uint64_t id;             // 令牌ID
    uint32_t capabilities;   // 能力位图
    uint32_t owner_pid;      // 拥有者PID
    uint32_t valid_until;    // 有效期（时间戳）
    char     description[64]; // 令牌描述
};

// 令牌函数
struct capability_token* acquire_capability(uint32_t capabilities, 
                                           const char* description,
                                           uint32_t timeout_sec);
int release_capability(struct capability_token* token);
int renew_capability(struct capability_token* token, uint32_t add_time_sec);
int validate_capability(struct capability_token* token, uint32_t required_caps);

// 虚拟端口令牌操作
int send_token_to_virtual_port(struct capability_token* token);
struct capability_token* receive_token_from_virtual_port(void);
int clear_virtual_port_tokens(void);

// 令牌验证
int check_capability(uint32_t required_caps);
int check_capability_for_pid(uint32_t required_caps, uint32_t pid);

#endif // ECLIB_CAPABILITY_H