#include "ebts/printkit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/time.h>

// 全局配置
static struct {
    LogOptions options;
    LogLevel min_level;
    char app_name[64];
    FILE* log_file;
    bool initialized;
    int message_count;
    struct timeval start_time;
} g_printkit = {
    .options = {
        .output_to_terminal = true,
        .output_to_syslog = true,
        .output_to_file = true,
        .add_timestamp = true,
        .add_process_info = true,
        .add_log_level = true,
        .flush_immediately = false
    },
    .min_level = LOG_LEVEL_INFO,
    .log_file = NULL,
    .initialized = false,
    .message_count = 0
};

int TerminalPrint_Init(const char* app_name) {
    if (g_printkit.initialized) {
        return 0; // 已初始化
    }
    
    // 保存应用名
    if (app_name) {
        strncpy(g_printkit.app_name, app_name, sizeof(g_printkit.app_name) - 1);
        g_printkit.app_name[sizeof(g_printkit.app_name) - 1] = '\0';
    } else {
        strcpy(g_printkit.app_name, "unknown");
    }
    
    // 打开系统日志
    if (g_printkit.options.output_to_syslog) {
        openlog(g_printkit.app_name, LOG_PID | LOG_CONS, LOG_USER);
    }
    
    // 打开日志文件
    if (g_printkit.options.output_to_file) {
        g_printkit.log_file = fopen("/var/log/ecomos/syslog.log", "a");
        if (!g_printkit.log_file) {
            // 尝试创建目录
            mkdir("/var/log/ecomos", 0755);
            g_printkit.log_file = fopen("/var/log/ecomos/syslog.log", "a");
        }
    }
    
    gettimeofday(&g_printkit.start_time, NULL);
    g_printkit.initialized = true;
    
    TerminalPrint_InfoF("TerminalPrint 系统初始化完成: %s", app_name);
    return 0;
}

static void format_message(
    char* buffer, 
    size_t buffer_size,
    LogLevel level,
    const char* message
) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    char time_buf[64];
    char level_str[16];
    pid_t pid = getpid();
    
    // 获取级别字符串
    switch (level) {
    case LOG_LEVEL_DEBUG:    strcpy(level_str, "DEBUG"); break;
    case LOG_LEVEL_INFO:     strcpy(level_str, "INFO"); break;
    case LOG_LEVEL_WARNING:  strcpy(level_str, "WARNING"); break;
    case LOG_LEVEL_ERROR:    strcpy(level_str, "ERROR"); break;
    case LOG_LEVEL_CRITICAL: strcpy(level_str, "CRITICAL"); break;
    default:                 strcpy(level_str, "UNKNOWN"); break;
    }
    
    // 格式化时间
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // 构建消息
    if (g_printkit.options.add_timestamp && g_printkit.options.add_process_info) {
        snprintf(buffer, buffer_size, 
                "[%s.%06ld][PID:%d][%s] %s",
                time_buf, tv.tv_usec,
                pid,
                level_str,
                message);
    } else if (g_printkit.options.add_timestamp) {
        snprintf(buffer, buffer_size,
                "[%s] %s",
                time_buf, message);
    } else if (g_printkit.options.add_process_info) {
        snprintf(buffer, buffer_size,
                "[PID:%d][%s] %s",
                pid, level_str, message);
    } else {
        snprintf(buffer, buffer_size, "%s", message);
    }
}

static void terminal_print_internal(
    LogLevel level,
    const char* message
) {
    if (!g_printkit.initialized) {
        // 自动初始化
        TerminalPrint_Init("auto-init");
    }
    
    if (level < g_printkit.min_level) {
        return; // 低于最小级别，不输出
    }
    
    g_printkit.message_count++;
    
    char formatted[1024];
    format_message(formatted, sizeof(formatted), level, message);
    
    // 1. 输出到终端
    if (g_printkit.options.output_to_terminal) {
        // 根据级别添加颜色
        const char* color_code = "";
        const char* reset_code = "\033[0m";
        
        switch (level) {
        case LOG_LEVEL_DEBUG:    color_code = "\033[36m"; break; // 青色
        case LOG_LEVEL_INFO:     color_code = "\033[32m"; break; // 绿色
        case LOG_LEVEL_WARNING:  color_code = "\033[33m"; break; // 黄色
        case LOG_LEVEL_ERROR:    color_code = "\033[31m"; break; // 红色
        case LOG_LEVEL_CRITICAL: color_code = "\033[35m"; break; // 洋红
        default:                 color_code = "";
        }
        
        fprintf(stdout, "%s%s%s\n", color_code, formatted, reset_code);
        
        if (g_printkit.options.flush_immediately) {
            fflush(stdout);
        }
    }
    
    // 2. 输出到系统日志
    if (g_printkit.options.output_to_syslog) {
        int syslog_level;
        switch (level) {
        case LOG_LEVEL_DEBUG:    syslog_level = LOG_DEBUG; break;
        case LOG_LEVEL_INFO:     syslog_level = LOG_INFO; break;
        case LOG_LEVEL_WARNING:  syslog_level = LOG_WARNING; break;
        case LOG_LEVEL_ERROR:    syslog_level = LOG_ERR; break;
        case LOG_LEVEL_CRITICAL: syslog_level = LOG_CRIT; break;
        default:                 syslog_level = LOG_INFO; break;
        }
        
        syslog(syslog_level, "[%s] %s", 
               g_printkit.app_name, message);
    }
    
    // 3. 输出到文件
    if (g_printkit.options.output_to_file && g_printkit.log_file) {
        fprintf(g_printkit.log_file, "%s\n", formatted);
        
        if (g_printkit.options.flush_immediately) {
            fflush(g_printkit.log_file);
        }
    }
}