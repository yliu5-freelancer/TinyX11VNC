#ifndef _CURSOR_INFO_H
#define _CURSOR_INFO_H

#include <stdint.h>

enum {
    CURSOR_SHAPE_ARROW,
    CURSOR_SHAPE_WATCH
};

typedef struct cursor_shape_t {
    uint8_t shape;
} cursor_shape_t;

typedef struct cursor_info_t {
    char *image_data;
    char *mask_data;
    int xpos;
    int ypos;
    cursor_shape_t now_shape;
    cursor_shape_t prev_shape;
} cursor_info_t;

#endif