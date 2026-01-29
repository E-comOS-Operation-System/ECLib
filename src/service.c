#include "eclib/service.h"
#include "eclib/ipc_message.h"
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h> // Added for snprintf

// Service registration table
static struct service_registry {
    struct service_info services[256];
    int count;
    int next_id;
} g_registry = {0};

int service_register(const char* name) {
    if (!name || strlen(name) >= 64) {
        return -1;
    }
    
    if (g_registry.count >= 256) {
        return -2;  // Registration table full
    }
    
    // Check if already registered
    for (int i = 0; i < g_registry.count; i++) {
        if (strcmp(g_registry.services[i].name, name) == 0) {
            return -3;  // Already exists
        }
    }
    
    // Register new service
    struct service_info* svc = &g_registry.services[g_registry.count];
    svc->id = g_registry.next_id++;
    strcpy(svc->name, name);
    svc->state = SERVICE_RUNNING;
    svc->pid = getpid();
    svc->start_time = time(NULL);
    svc->last_heartbeat = svc->start_time;
    
    g_registry.count++;
    
    // Send registration message
    char data[128];
    snprintf(data, sizeof(data), "SERVICE_REGISTER:%s:%d", name, svc->id);
    ipc_broadcast_msg(0x53455256, 0, strlen(data), data);  // "SERV"
    
    return svc->id;
}

int service_unregister(int service_id) {
    for (int i = 0; i < g_registry.count; i++) {
        if (g_registry.services[i].id == service_id) {
            // Send unregistration message
            char data[128];
            snprintf(data, sizeof(data), "SERVICE_UNREGISTER:%d", service_id);
            ipc_broadcast_msg(0x53455256, 0, strlen(data), data);
            
            // Remove
            for (int j = i; j < g_registry.count - 1; j++) {
                g_registry.services[j] = g_registry.services[j + 1];
            }
            g_registry.count--;
            return 0;
        }
    }
    return -1;  // Not found
}

int service_heartbeat(int service_id) {
    for (int i = 0; i < g_registry.count; i++) {
        if (g_registry.services[i].id == service_id) {
            g_registry.services[i].last_heartbeat = time(NULL);
            return 0;
        }
    }
    return -1;
}