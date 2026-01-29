#ifndef EBTS_PRINTKIT_H
#define EBTS_PRINTKIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h> // Added for size_t

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG = 0,     // Debug information
    LOG_LEVEL_INFO,         // Informational messages
    LOG_LEVEL_WARNING,      // Warnings
    LOG_LEVEL_ERROR,        // Errors
    LOG_LEVEL_CRITICAL      // Critical errors
} LogLevel;

// Log output options
typedef struct {
    bool output_to_terminal;    // Output to terminal
    bool output_to_syslog;      // Output to system log
    bool output_to_file;        // Output to file
    bool add_timestamp;         // Add timestamp
    bool add_process_info;      // Add process information
    bool add_log_level;         // Add log level
    bool flush_immediately;     // Flush immediately
} LogOptions;

// Basic print functions
void TerminalPrint_Debug(const char* message);
void TerminalPrint_Info(const char* message);
void TerminalPrint_Warning(const char* message);
void TerminalPrint_Error(const char* message);
void TerminalPrint_Critical(const char* message);

// Formatted versions
void TerminalPrint_DebugF(const char* format, ...);
void TerminalPrint_InfoF(const char* format, ...);
void TerminalPrint_WarningF(const char* format, ...);
void TerminalPrint_ErrorF(const char* format, ...);
void TerminalPrint_CriticalF(const char* format, ...);

// Variadic version
void TerminalPrint_V(LogLevel level, const char* format, va_list args);

// Control functions
void TerminalPrint_SetOptions(const LogOptions* options);
void TerminalPrint_GetOptions(LogOptions* options);
void TerminalPrint_SetLevel(LogLevel min_level);
LogLevel TerminalPrint_GetLevel(void);

// Initialization/cleanup
int TerminalPrint_Init(const char* app_name);
void TerminalPrint_Cleanup(void);

// Utility functions
void TerminalPrint_HexDump(const char* label, const void* data, size_t length);
void TerminalPrint_MemoryStats(void);

// Appendix S specific
void TerminalPrint_ServiceSaveInfo(void);
void TerminalPrint_AppendixS(const char* action, int pid, int time_ms);

#endif // EBTS_PRINTKIT_H