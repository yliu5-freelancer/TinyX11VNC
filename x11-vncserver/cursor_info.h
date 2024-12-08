#ifndef _CURSOR_INFO_H
#define _CURSOR_INFO_H

#include <stdint.h>

enum {
    CURSOR_SHAPE_ARROW,
    CURSOR_SHAPE_WATCH
};

typedef struct {
    uint8_t shape;
} cursor_shape_t;

typedef struct {
    char *image_data;
    char *mask_data;
    int xpos;
    int ypos;
    cursor_shape_t now_shape;
    cursor_shape_t prev_shape;
} cursor_info_t;
#endif