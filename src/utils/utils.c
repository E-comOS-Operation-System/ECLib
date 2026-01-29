/*
 * ECLib - E-comOS C Library
 * Copyright (C) 2025 E-comOS Kernel Mode Team & Saladin5101
 * 
 * This file is part of ECLib.
 * ECLib is free library; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */
#include "eclib/utils.h"
#include "eclib/error.h"
#include "eclib/ipc_message.h"
#include "eclib/men.h"
#include <stdarg.h>
// --------------------------
// String
// --------------------------
size_t eclib_strlen(const char* str) {
    if (str == NULL) return 0;
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char* eclib_strncpy(char* dest, const char* src, size_t max_len) {
    if (dest == NULL || src == NULL || max_len == 0) {
        return dest;
    }
    size_t i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0'; 
    return dest;
}

int eclib_strcmp(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL) return -1;
    if (str2 == NULL) return 1;

    while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
        str1++;
        str2++;
    }
    return (int)(*str1 - *str2);
}

const char* eclib_strstr(const char* str, const char* substr) {
    if (str == NULL || substr == NULL || *substr == '\0') {
        return NULL;
    }
    size_t substr_len = eclib_strlen(substr);
    size_t str_len = eclib_strlen(str);

    if (substr_len > str_len) {
        return NULL;
    }

    for (size_t i = 0; i <= str_len - substr_len; i++) {
        int match = 1;
        for (size_t j = 0; j < substr_len; j++) {
            if (str[i + j] != substr[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            return &str[i];
        }
    }
    return NULL;
}

// --------------------------
// Mentory
// --------------------------
void* eclib_memcpy(void* dest, const void* src, size_t len) {
    if (dest == NULL || src == NULL || len == 0) {
        return dest;
    }
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* eclib_memmove(void* dest, const void* src, size_t len) {
    if (dest == NULL || src == NULL || len == 0) {
        return dest;
    }
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

   
    if (d > s && d < s + len) {
        for (size_t i = len; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    } else {
        
        eclib_memcpy(d, s, len);
    }
    return dest;
}

void* eclib_memset(void* ptr, int value, size_t len) {
    if (ptr == NULL || len == 0) {
        return ptr;
    }
    uint8_t* p = (uint8_t*)ptr;
    uint8_t val = (uint8_t)value;
    for (size_t i = 0; i < len; i++) {
        p[i] = val;
    }
    return ptr;
}

// --------------------------
// Char
// --------------------------
int eclib_isdigit(int c) {
    return (c >= '0' && c <= '9') ? 1 : 0;
}

int eclib_isalpha(int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? 1 : 0;
}

int eclib_tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int eclib_toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

int eclib_strncmp(const char* str1, const char* str2, size_t n) {
    if (!str1 || !str2) return str1 ? 1 : (str2 ? -1 : 0);
    for (size_t i = 0; i < n; i++) {
        if (str1[i] != str2[i] || str1[i] == '\0')
            return (unsigned char)str1[i] - (unsigned char)str2[i];
    }
    return 0;
}

char* eclib_strchr(const char* s, int c) {
    if (!s) return 0;
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == '\0') ? (char*)s : 0;
}

char* eclib_strrchr(const char* s, int c) {
    if (!s) return 0;
    const char* last = 0;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == '\0') return (char*)s;
    return (char*)last;
}

char* eclib_strdup(const char* s) {
    if (!s) return 0;
    size_t len = eclib_strlen(s);
    char* dup = eclib_malloc(len + 1);
    if (dup) eclib_memcpy(dup, s, len + 1);
    return dup;
}

static char* strtok_state = 0;

char* eclib_strtok(char* str, const char* delim) {
    if (str) strtok_state = str;
    if (!strtok_state) return 0;
    
    while (*strtok_state && eclib_strchr(delim, *strtok_state))
        strtok_state++;
    
    if (!*strtok_state) return 0;
    
    char* token = strtok_state;
    while (*strtok_state && !eclib_strchr(delim, *strtok_state))
        strtok_state++;
    
    if (*strtok_state) {
        *strtok_state = '\0';
        strtok_state++;
    }
    
    return token;
}

int eclib_snprintf(char* buf, size_t size, const char* fmt, ...) {
    if (!buf || !size || !fmt) return -1;
    
    va_list args;
    va_start(args, fmt);
    
    size_t pos = 0;
    while (*fmt && pos < size - 1) {
        if (*fmt == '%' && *(fmt + 1)) {
            fmt++;
            if (*fmt == 's') {
                const char* s = va_arg(args, const char*);
                if (s) {
                    while (*s && pos < size - 1)
                        buf[pos++] = *s++;
                }
            } else if (*fmt == 'd') {
                int val = va_arg(args, int);
                char tmp[32];
                int i = 0, neg = val < 0;
                if (neg) val = -val;
                do { tmp[i++] = '0' + (val % 10); val /= 10; } while (val);
                if (neg && pos < size - 1) buf[pos++] = '-';
                while (i > 0 && pos < size - 1) buf[pos++] = tmp[--i];
            } else if (*fmt == 'c') {
                buf[pos++] = (char)va_arg(args, int);
            } else {
                buf[pos++] = *fmt;
            }
        } else {
            buf[pos++] = *fmt;
        }
        fmt++;
    }
    buf[pos] = '\0';
    
    va_end(args);
    return pos;
}