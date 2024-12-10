#ifndef _SCREEN_INFO_H
#define _SCREEN_INFO_H

#include <stdint.h>
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>

#define TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS 4
#define DEFAULT_SCREEN_NUM 0

enum xcb_image_pixfmt_t {
    XCB_PIXFMT_RGB = 127,
    XCB_PIXFMT_BGR
};
typedef struct screen_pixfmt_t {
    enum xcb_image_pixfmt_t pixfmt;
} screen_pixfmt_t;

typedef struct screen_info_t {
    int dmg_xpos;
    int dmg_ypos;
    int dmg_area_width;
    int dmg_area_height;
    int default_width;
    int default_height;
    int scaled_width;
    int scaled_height;
    float scale_factor;
    xcb_screen_t *screen;
    xcb_window_t root_window;
    cursor_info_t root_cursor;
    screen_pixfmt_t screen_pixfmt;
    uint8_t *fbdata;
} screen_info_t;

typedef struct display_info_t {
    int screen_nums;
    char *display_name;
    xcb_connection_t *connection;
    screen_info_t screens[TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS];
} display_info_t;


#endif