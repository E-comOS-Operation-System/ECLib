/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 */
#include "eclib/env.h"
#include "eclib/utils.h"
#include "eclib/men.h"

char** eclib_environ = 0;

char* eclib_getenv(const char* name) {
    if (!name || !eclib_environ) return 0;
    
    size_t name_len = eclib_strlen(name);
    
    for (int i = 0; eclib_environ[i]; i++) {
        if (eclib_strncmp(eclib_environ[i], name, name_len) == 0 &&
            eclib_environ[i][name_len] == '=') {
            return &eclib_environ[i][name_len + 1];
        }
    }
    
    return 0;
}

int eclib_setenv(const char* name, const char* value, int overwrite) {
    if (!name || !value || eclib_strchr(name, '=')) return -1;
    
    size_t name_len = eclib_strlen(name);
    size_t value_len = eclib_strlen(value);
    
    // Find existing
    int idx = -1;
    int count = 0;
    if (eclib_environ) {
        for (int i = 0; eclib_environ[i]; i++) {
            if (eclib_strncmp(eclib_environ[i], name, name_len) == 0 &&
                eclib_environ[i][name_len] == '=') {
                if (!overwrite) return 0;
                idx = i;
            }
            count++;
        }
    }
    
    char* new_entry = eclib_malloc(name_len + value_len + 2);
    if (!new_entry) return -1;
    
    eclib_memcpy(new_entry, name, name_len);
    new_entry[name_len] = '=';
    eclib_memcpy(new_entry + name_len + 1, value, value_len + 1);
    
    if (idx >= 0) {
        eclib_free(eclib_environ[idx]);
        eclib_environ[idx] = new_entry;
    } else {
        char** new_environ = eclib_malloc((count + 2) * sizeof(char*));
        if (!new_environ) {
            eclib_free(new_entry);
            return -1;
        }
        
        for (int i = 0; i < count; i++) {
            new_environ[i] = eclib_environ[i];
        }
        new_environ[count] = new_entry;
        new_environ[count + 1] = 0;
        
        eclib_free(eclib_environ);
        eclib_environ = new_environ;
    }
    
    return 0;
}

int eclib_unsetenv(const char* name) {
    if (!name || !eclib_environ) return -1;
    
    size_t name_len = eclib_strlen(name);
    
    for (int i = 0; eclib_environ[i]; i++) {
        if (eclib_strncmp(eclib_environ[i], name, name_len) == 0 &&
            eclib_environ[i][name_len] == '=') {
            eclib_free(eclib_environ[i]);
            
            // Shift remaining entries
            for (int j = i; eclib_environ[j]; j++) {
                eclib_environ[j] = eclib_environ[j + 1];
            }
            return 0;
        }
    }
    
    return 0;
}
