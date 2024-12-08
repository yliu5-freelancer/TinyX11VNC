#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/limits.h>

#include <rfb/rfb.h>
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>

#include "cursor_image.h"
#include "cursor_info.h"

#define VNCSERVER_PORT 5900
#define HTTP_VNCSERVER_PORT 5800
#define DESKTOP_NAME "Simple VNC Desktop"
#define VNCSERVER_SECRET "ca$hc0w"
#define VNCSERVER_SECRET_PATH "/tmp/"
#define VNCSERVER_BUTTON_MASK_LEFT        rfbButton1Mask
#define VNCSeRVER_BUTTON_MASK_MIDDLE      rfbButton2Mask
#define VNCSERVER_BUTTON_MASK_RIGHT       rfbButton3Mask
#define VNCSERVER_BUTTON_MASK_UP          rfbWheelUpMask
#define VNCSERVER_BUTTON_MASK_DOWN        rfbWheelDownMask

enum xcb_image_pixfmt_t {
    XCB_PIXFMT_RGB = 127,
    XCB_PIXFMT_BGR
};

rfbScreenInfoPtr rfb_screen = NULL;
xcb_connection_t *connection;
xcb_screen_t *screen;
xcb_window_t root_window;
cursor_info_t root_cursor;

void
client_disconnection(rfbClientPtr client)
{
    free(client->clientData);
    client->clientData = NULL;
}

enum rfbNewClientAction
get_new_client_connection(rfbClientPtr client)
{
    client->clientData = NULL;
    client->clientGoneHook = client_disconnection;
    return RFB_CLIENT_ACCEPT;
}

static void
send_button_to_xserver(xcb_connection_t *connection,
                       xcb_button_t button,
                       bool is_pressed)
{
    xcb_test_fake_input(connection, is_pressed ? XCB_BUTTON_PRESS : XCB_BUTTON_RELEASE, button, XCB_CURRENT_TIME, XCB_NONE, 0, 0, 0);
    xcb_flush(connection);
}

static void
send_motion_to_xserver(xcb_connection_t *connection,
                       int x,
                       int y)
{
    xcb_test_fake_input(connection, XCB_MOTION_NOTIFY, 0, XCB_CURRENT_TIME, XCB_NONE, x, y, 0);
    xcb_flush(connection);
}

static void
send_keysym_to_xserver(xcb_connection_t *connection,
                       xcb_keysym_t key_sym,
                       int is_pressd)
{
    xcb_key_symbols_t *symbols = xcb_key_symbols_alloc(connection);
    xcb_keycode_t* code = xcb_key_symbols_get_keycode(symbols, key_sym);
    while (code && *code != XCB_NO_SYMBOL) {
        xcb_test_fake_input(connection, is_pressd ? XCB_KEY_PRESS : XCB_KEY_RELEASE, *code,
            XCB_CURRENT_TIME, XCB_NONE, 0, 0, 0);
        xcb_flush(connection);
        code++;
    }
}

static bool
mouse_cursor_updated(cursor_shape_t now,
                     cursor_shape_t prev)
{
    return now.shape == prev.shape;
}

static void
send_cursor_image_to_vncclient(xcb_connection_t *connection)
{
#if 0
    xcb_generic_error_t *error = NULL;  

    uint8_t major_version = 5;
    uint8_t minor_version = 0;
    xcb_xfixes_query_version_cookie_t version_cookie;
    xcb_xfixes_query_version_reply_t *version_reply;
    xcb_xfixes_get_cursor_image_cookie_t cookie;
    xcb_xfixes_get_cursor_image_reply_t *cursor_image_reply;

    version_cookie = xcb_xfixes_query_version(connection, major_version, minor_version);
    version_reply = xcb_xfixes_query_version_reply(connection, version_cookie, &error);
    if (!version_reply) {
        printf("XFixes version: %d.%d\n", version_reply->major_version, version_reply->minor_version);
        free(version_reply);
        return;
    }
    cookie = xcb_xfixes_get_cursor_image(connection);
    cursor_image_reply = xcb_xfixes_get_cursor_image_reply(connection, cookie, &error);
    if (!cursor_image_reply) {
        fprintf(stderr, "XCB Error occurred! Error code: (%d)\n", error->error_code);
        free(cursor_image_reply);
        return;
    }

#endif
    rfbCursorPtr cursor_ptr = rfbMakeXCursor(18, 18, cursor_arrow_data, cursor_arrow_mask);
    rfbMakeRichCursorFromXCursor(rfb_screen, cursor_ptr);
    rfbSetCursor(rfb_screen, cursor_ptr);
#if 0
    free(cursor_image_data);
    free(mask_image_data);
    free(version_reply);
    free(cursor_image_reply);
#endif
}


static void
pointer_event_callback(int btn_mask,
                       int x,
                       int y,
                       rfbClientPtr client)
{
#ifdef DEBUG
    printf("The mouse position is x, y: %d, %d, %#x\n", x, y, btn_mask);
#endif
    send_button_to_xserver(connection, XCB_BUTTON_INDEX_1, !!(btn_mask & VNCSERVER_BUTTON_MASK_LEFT));
    send_button_to_xserver(connection, XCB_BUTTON_INDEX_2, !!(btn_mask & VNCSeRVER_BUTTON_MASK_MIDDLE));
    send_button_to_xserver(connection, XCB_BUTTON_INDEX_3, !!(btn_mask & VNCSERVER_BUTTON_MASK_RIGHT));
    send_button_to_xserver(connection, XCB_BUTTON_INDEX_4, !!(btn_mask & VNCSERVER_BUTTON_MASK_UP));
    send_button_to_xserver(connection, XCB_BUTTON_INDEX_5, !!(btn_mask & VNCSERVER_BUTTON_MASK_DOWN));
    send_motion_to_xserver(connection, x, y);
    if (!mouse_cursor_updated(root_cursor.prev_shape, root_cursor.now_shape)) {
        send_cursor_image_to_vncclient(connection);
    }
}

static void
keyboard_event_callback(rfbBool down,
                        rfbKeySym key_sym,
                        rfbClientPtr client)
{
    send_keysym_to_xserver(connection, key_sym, down);
}

int
init_vnc_server(int argc,
                char *argv[],
                rfbScreenInfoPtr *rfb_screen,
                int width,
                int height,
                const char *secret,
                const char *secret_store,
                uint16_t vncport,
                uint16_t http_vncport)
{
    rfbEncryptAndStorePasswd((char *) secret, (char *) secret_store);
    *rfb_screen = rfbGetScreen(&argc, argv, width, height, 8, 3, 4);
    (*rfb_screen)->frameBuffer = (char *) malloc(width * height * 4);
    if (!(*rfb_screen)->frameBuffer) {
        fprintf(stderr, "Initialize the frame buffer failed.\n");
        return -1;
    }
    (*rfb_screen)->desktopName = DESKTOP_NAME;
    (*rfb_screen)->alwaysShared = TRUE;  // It allows more different client get connection.
    (*rfb_screen)->width = width;
    (*rfb_screen)->height = height;
    (*rfb_screen)->port = vncport;
    (*rfb_screen)->newClientHook = get_new_client_connection;
    (*rfb_screen)->authPasswdData = (void*) secret_store;
    (*rfb_screen)->httpDir = "./httpvnc";
    (*rfb_screen)->httpPort = http_vncport;
    (*rfb_screen)->httpEnableProxyConnect = TRUE;
    (*rfb_screen)->ptrAddEvent = pointer_event_callback;
    (*rfb_screen)->kbdAddEvent = keyboard_event_callback;
    rfbInitServer(*rfb_screen);
}

enum xcb_image_pixfmt_t
detect_pixfmt(xcb_connection_t *connection, xcb_screen_t *screen)
{
    xcb_depth_iterator_t depth_iter;

    depth_iter = xcb_screen_allowed_depths_iterator(screen);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
        xcb_visualtype_iterator_t visual_iter;
        visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next (&visual_iter)) {
        #ifdef DEBUG
            printf("%#x, %#x\n", screen->root_visual, visual_iter.data->visual_id);
        #endif
            if (screen->root_visual == visual_iter.data->visual_id) {
                xcb_visualtype_t *visual_type = visual_iter.data;
                if (visual_type) {
                    if ((visual_type->red_mask >> 16) == 0xff &&
                        (visual_type->green_mask >> 8) == 0xff &&
                        (visual_type->blue_mask == 0xff)) {
                        return XCB_PIXFMT_RGB;
                    } else if ((visual_type->blue_mask >> 16) == 0xff &&
                               (visual_type->green_mask >> 8) == 0xff &&
                               (visual_type->red_mask == 0xff)) {
                        return XCB_PIXFMT_BGR;            
                    }
                    return -1;
                }
                break;
            }
        }
    }
    return -1;
}

void
capture_screen(xcb_connection_t *connection,
               xcb_window_t window,
               xcb_screen_t *screen,
               char *framebuffer,
               int screen_width,
               int screen_height,
               enum xcb_image_pixfmt_t color_seq)
{

    xcb_get_image_cookie_t image_cookie = xcb_get_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP,  window,
        0, 0, screen_width, screen_height, ~0);
    xcb_get_image_reply_t *image_reply = xcb_get_image_reply(connection, image_cookie, NULL);
    unsigned char *image_data = xcb_get_image_data(image_reply);

    for (int i = 0; i < screen_height; i++) {
        for (int j = 0; j < screen_width; j++) {
            unsigned long pixel = *((unsigned long *)(image_data + (i * screen_width + j) * 4));
            unsigned char red, green, blue, alpha;
            unsigned int true_pixel;
            if (color_seq == XCB_PIXFMT_RGB) {
                red = pixel & 0xff;
                blue = (pixel >> 16) & 0xff;
            } else if (color_seq == XCB_PIXFMT_BGR) {
                red = (pixel >> 16) & 0xff;
                blue = pixel & 0xff;
            } else {
                return;
            }
            green = (pixel >> 8) & 0xFF;
            alpha = (pixel >> 24) & 0xFF;
            true_pixel = (red << 16) | (green << 8) | (blue) | (alpha << 24);
            ((unsigned int *)framebuffer)[i * screen_width + j] = true_pixel;
        }
    }
    free(image_reply);
}

void
setup_cursor()
{
    root_cursor.image_data = cursor_arrow_data;
    root_cursor.mask_data = cursor_arrow_mask;
    root_cursor.now_shape.shape = CURSOR_SHAPE_ARROW;
    root_cursor.prev_shape.shape = -1;
}

int main(int argc, char *argv[])
{
    enum xcb_image_pixfmt_t color_seq;
    int screen_width = 0, screen_height = 0;
    int screen_num = 0;
    char secret_file[PATH_MAX] = {};

    connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection)) {
        fprintf(stderr, "Can not connect to XServer.\n");
        return -1;
    }
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    root_window = screen->root;
    screen_width = screen->width_in_pixels;
    screen_height = screen->height_in_pixels;
    snprintf(secret_file, PATH_MAX, "%s%s.%d", VNCSERVER_SECRET_PATH, "secret", getpid());
    color_seq = detect_pixfmt(connection, screen);
    if (color_seq < 0) {
        return -1;
    }

    setup_cursor();    
    init_vnc_server(argc, argv, &rfb_screen, screen_width, screen_height, 
        VNCSERVER_SECRET, secret_file, VNCSERVER_PORT, HTTP_VNCSERVER_PORT);

    rfbRunEventLoop(rfb_screen, 1000, TRUE);
    while (TRUE) {
        capture_screen(connection, root_window, screen, rfb_screen->frameBuffer, screen_width, screen_height, color_seq);
        rfbMarkRectAsModified(rfb_screen, 0, 0, screen_width, screen_height);
    }
    free(rfb_screen->frameBuffer);
    xcb_disconnect(connection);
    return 0;
}