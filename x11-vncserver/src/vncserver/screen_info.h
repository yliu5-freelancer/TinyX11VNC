#ifndef _SCREEN_INFO_H
#define _SCREEN_INFO_H

#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>
#include <xcb/damage.h>

#define TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS 4
#define DEFAULT_SCREEN_NUM 0

enum xcb_image_pixfmt_t {
    XCB_PIXFMT_RGB = 127,
    XCB_PIXFMT_BGR
};
typedef struct screen_pixfmt_t {
    enum xcb_image_pixfmt_t pixfmt;
} screen_pixfmt_t;

typedef struct window_damage_info_t {
    int damage_xpos;
    int damage_ypos;
    int damage_area_width;
    int damage_area_height;
    uint8_t damage_event;
    xcb_void_cookie_t window_damage_cookie;
} window_damage_info_t;
typedef struct screen_info_t {
    int default_width;
    int default_height;
    int scaled_width;
    int scaled_height;
    float scale_factor;
    xcb_screen_t *screen;
    xcb_window_t root_window;
    cursor_info_t root_cursor;
    screen_pixfmt_t screen_pixfmt;
    window_damage_info_t window_damage;
    uint8_t *fbdata;
} screen_info_t;

typedef struct display_info_t {
    int screen_nums;
    char *display_name;
    xcb_connection_t *connection;
    screen_info_t screens[TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS];
    xcb_generic_event_t *xcb_event;
    xcb_damage_damage_t damage_id;
    xcb_damage_query_version_cookie_t damage_cookie;
    xcb_damage_query_version_reply_t *damage_version_reply;
    uint32_t damage_report_level;
} display_info_t;

#endif