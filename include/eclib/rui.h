#ifndef ECLIB_RUI_H
#define ECLIB_RUI_H

#include "eclib/ipc_message.h"
#include "eclib/error.h"
#include <stdint.h>
#include <stddef.h>

// RUI Command Codes
#define RUI_CMD_WINDOW_CREATE    0x3001
#define RUI_CMD_WINDOW_CLOSE     0x3002
#define RUI_CMD_DRAW_TEXT        0x3003
#define RUI_CMD_DRAW_BUTTON      0x3004
#define RUI_CMD_EVENT_REGISTER   0x3005

// RUI Event Codes
#define RUI_EVENT_CLICK          0x4001
#define RUI_EVENT_KEYDOWN        0x4002
#define RUI_EVENT_WINDOW_CLOSE   0x4003

// Data Types
typedef struct {
    int16_t x;
    int16_t y;
} rui_point_t;

typedef struct {
    uint16_t width;
    uint16_t height;
} rui_size_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rui_color_t;

// Request Structures
typedef struct {
    rui_point_t pos;
    rui_size_t size;
    char title[64];
    rui_color_t bg_color;
} rui_window_create_req_t;

typedef struct {
    uint32_t window_id;
    eclib_err_t err;
} rui_window_create_resp_t;

typedef struct {
    uint32_t window_id;
    rui_point_t pos;
    char text[256];
    rui_color_t color;
    uint8_t font_size;
} rui_draw_text_req_t;

// Event Structures
typedef struct {
    uint32_t window_id;
    rui_point_t pos;
    uint8_t button;
} rui_click_event_t;

typedef struct {
    uint32_t window_id;
    char key;
    uint8_t is_ctrl;
} rui_keydown_event_t;

typedef struct {
    uint16_t event_type;
    union {
        rui_click_event_t click;
        rui_keydown_event_t keydown;
    } data;
} rui_event_t;

// Function Prototypes
eclib_err_t eclib_rui_init(void);
uint32_t eclib_rui_create_window(const rui_point_t* pos, const rui_size_t* size, const char* title, const rui_color_t* bg_color);
eclib_err_t eclib_rui_close_window(uint32_t window_id);
eclib_err_t eclib_rui_draw_text(uint32_t window_id, const rui_point_t* pos, const char* text, const rui_color_t* color, uint8_t font_size);
eclib_err_t eclib_rui_register_event(uint32_t window_id, uint16_t event_type);
eclib_err_t eclib_rui_wait_event(rui_event_t* event, uint32_t timeout_ms);

#endif // ECLIB_RUI_H