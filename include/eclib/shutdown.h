#ifndef ECLIB_SHUTDOWN_H
#define ECLIB_SHUTDOWN_H

#include <stdint.h> // Added for uint32_t

// Shutdown reasons
enum shutdown_reason {
    SHUTDOWN_NORMAL = 0,      // Normal shutdown
    SHUTDOWN_REBOOT,         // Reboot
    SHUTDOWN_EMERGENCY,      // Emergency shutdown
    SHUTDOWN_USER_REQUEST,   // User request
    SHUTDOWN_POWER_FAILURE,  // Power failure
    SHUTDOWN_THERMAL,        // Overheat
    SHUTDOWN_PANIC,         // Kernel panic
};


enum shutdown_flags {
    SHUTDOWN_FLAG_GRACEFUL = 1 << 0,  
    SHUTDOWN_FLAG_FORCE    = 1 << 1,  
    SHUTDOWN_FLAG_NOWAIT   = 1 << 2,  
    SHUTDOWN_FLAG_NOLOG    = 1 << 3,  
};

int shutdown_system(enum shutdown_reason reason, uint32_t flags, 
                    const char* message);
int reboot_system(enum shutdown_reason reason, uint32_t flags,
                  const char* message);

typedef void (*shutdown_callback_t)(enum shutdown_reason reason, 
                                    void* user_data);
int register_shutdown_callback(shutdown_callback_t callback, void* user_data);
int unregister_shutdown_callback(shutdown_callback_t callback);

int get_shutdown_status(void);
int cancel_shutdown(void);

int appendix_s_begin_save(void);
int appendix_s_save_complete(void);
int appendix_s_get_remaining_time(void);

#endif // ECLIB_SHUTDOWN_H