#ifndef _CURSOR_INFO_H
#define _CURSOR_INFO_H

#include <stdint.h>

enum cursor_shape {
    CURSOR_SHAPE_ARROW,
    CURSOR_SHAPE_WATCH,
    CURSOR_SHAPE_HAND,
    CURSOR_SHAPE_TEXT,
    CURSOR_SHAPE_FLEUR,
    CURSOR_SHAPE_ULRESIZE,
    CURSOR_SHAPE_URESIZE,
    CURSOR_SHAPE_URRESIZE,
    CURSOR_SHAPE_LRESIZE,
    CURSOR_SHAPE_RRESIZE,
    CURSOR_SHAPE_BLRESIZE,
    CURSOR_SHAPE_BRESIZE,
    CURSOR_SHAPE_BRRESIZE,
    CURSOR_SHAPE_PLUS,
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