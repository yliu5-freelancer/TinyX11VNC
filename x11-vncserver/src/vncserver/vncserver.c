#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <getopt.h>
#include <libgen.h>
#include <assert.h>

#include <rfb/rfb.h>
#include <xcb/xcb.h>
#include <xcb/xtest.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>
#include <xcb/damage.h>

#include "cursor_image.h"
#include "cursor_info.h"
#include "screen_info.h"
#include "scale_framebuffer.h"

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define VNCSERVER_PORT 5900
#define HTTP_VNCSERVER_PORT 5800
#define DESKTOP_NAME "Tiny VNC Desktop"
#define VNCSERVER_SECRET "ca$hc0w"
#define VNCSERVER_SECRET_PATH "/tmp/"
#define VNCSERVER_BUTTON_MASK_LEFT        rfbButton1Mask
#define VNCSeRVER_BUTTON_MASK_MIDDLE      rfbButton2Mask
#define VNCSERVER_BUTTON_MASK_RIGHT       rfbButton3Mask
#define VNCSERVER_BUTTON_MASK_UP          rfbWheelUpMask
#define VNCSERVER_BUTTON_MASK_DOWN        rfbWheelDownMask

rfbScreenInfoPtr g_rfbscreen_ptr[TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS] = {NULL};
display_info_t *g_xcbscreen_ptr = NULL;
char g_secret_store[PATH_MAX] = {};

void
print_usage(int argc, char *argv[])
{
    fprintf(stderr, "Usage: %s [-d display]\n", basename(argv[0]));
}

void
client_gone_callback(rfbClientPtr client)
{
    free(client->clientData);
    client->clientData = NULL;
}

void
client_fb_update_request_callback(rfbClientPtr client, rfbFramebufferUpdateRequestMsg *furMsg)
{
}

enum rfbNewClientAction
get_new_client_connection(rfbClientPtr client)
{
    client->clientData = NULL;
    client->clientGoneHook = client_gone_callback;
    client->clientFramebufferUpdateRequestHook = client_fb_update_request_callback;
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
send_cursor_image_to_vncclient(int which)
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
    rfbMakeRichCursorFromXCursor(g_rfbscreen_ptr[which], cursor_ptr);
    rfbSetCursor(g_rfbscreen_ptr[which], cursor_ptr);
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
    send_button_to_xserver(g_xcbscreen_ptr->connection,
        XCB_BUTTON_INDEX_1, !!(btn_mask & VNCSERVER_BUTTON_MASK_LEFT));
    send_button_to_xserver(g_xcbscreen_ptr->connection,
        XCB_BUTTON_INDEX_2, !!(btn_mask & VNCSeRVER_BUTTON_MASK_MIDDLE));
    send_button_to_xserver(g_xcbscreen_ptr->connection,
        XCB_BUTTON_INDEX_3, !!(btn_mask & VNCSERVER_BUTTON_MASK_RIGHT));
    send_button_to_xserver(g_xcbscreen_ptr->connection,
        XCB_BUTTON_INDEX_4, !!(btn_mask & VNCSERVER_BUTTON_MASK_UP));
    send_button_to_xserver(g_xcbscreen_ptr->connection,
        XCB_BUTTON_INDEX_5, !!(btn_mask & VNCSERVER_BUTTON_MASK_DOWN));
    send_motion_to_xserver(g_xcbscreen_ptr->connection, x, y);
    if (!mouse_cursor_updated(
        g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].root_cursor.now_shape,
        g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].root_cursor.prev_shape)) {
        send_cursor_image_to_vncclient(DEFAULT_SCREEN_NUM);
    }
}

static void
keyboard_event_callback(rfbBool down,
                        rfbKeySym key_sym,
                        rfbClientPtr client)
{
    send_keysym_to_xserver(g_xcbscreen_ptr->connection, key_sym, down);
}

static int
desktopsize_event_callback(int width,
                           int height,
                           int numScreens,
                           struct rfbExtDesktopScreen* extDesktopScreens,
                           struct _rfbClientRec* cl)
{
    fprintf(stdout, "The new event desktop size is: (%d, %d)\n", width, height);
    return 0;
}

static rfbBool
ext_desktop_screen_event_callback(int seqnumber,
                                  struct rfbExtDesktopScreen *extDesktopScreen,
                                  struct _rfbClientRec* cl)
{
    return TRUE;
}

enum xcb_image_pixfmt_t
detect_screen_pixfmt(xcb_screen_t *screen)
{
    return XCB_PIXFMT_RGB;
    xcb_depth_iterator_t depth_iter;

    depth_iter = xcb_screen_allowed_depths_iterator(screen);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
        xcb_visualtype_iterator_t visual_iter;
        visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
        // #ifdef DEBUG
            printf("%#x, %#x\n", screen->root_visual, visual_iter.data->visual_id);
        // #endif
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
capture_screen(int which)
{
    screen_info_t screen;
    int capture_width, capture_height;
    int capture_xpos, capture_ypos;

    screen = g_xcbscreen_ptr->screens[which];
    capture_width = screen.window_damage.damage_area_width == 0 ? screen.default_width : screen.window_damage.damage_area_width;
    capture_height = screen.window_damage.damage_area_height == 0 ? screen.default_height : screen.window_damage.damage_area_height;
    capture_xpos = screen.window_damage.damage_xpos;
    capture_ypos = screen.window_damage.damage_ypos;

    xcb_get_image_cookie_t image_cookie = xcb_get_image(g_xcbscreen_ptr->connection, 
        XCB_IMAGE_FORMAT_Z_PIXMAP, screen.root_window,
        capture_xpos, capture_ypos, capture_width, capture_height, ~0);
    xcb_get_image_reply_t *image_reply = xcb_get_image_reply(g_xcbscreen_ptr->connection,
        image_cookie, NULL);
    unsigned char *image_data = xcb_get_image_data(image_reply);
   
    for (int i = capture_ypos; i < capture_height; i++) {
        for (int j = capture_xpos; j < capture_width; j++) {
            unsigned long pixel = 
                *((unsigned long *)(image_data + (i * capture_width + j) * 4));
            unsigned char red, green, blue, alpha;
            unsigned int true_pixel;

            if (screen.screen_pixfmt.pixfmt == XCB_PIXFMT_RGB) {
                red = pixel & 0xff;
                blue = (pixel >> 16) & 0xff;
            } else if (screen.screen_pixfmt.pixfmt == XCB_PIXFMT_BGR) {
                red = (pixel >> 16) & 0xff;
                blue = pixel & 0xff;
            } else {
                return;
            }
            green = (pixel >> 8) & 0xFF;
            alpha = (pixel >> 24) & 0xFF;
            true_pixel = (red << 16) | (green << 8) | (blue) | (alpha << 24);
            ((unsigned int *)screen.fbdata)[i * capture_width + j] = true_pixel;
        }
    }
    free(image_reply);
    g_rfbscreen_ptr[which]->frameBuffer = screen.fbdata;
    rfbMarkRectAsModified(g_rfbscreen_ptr[which], capture_xpos, capture_ypos, capture_width, capture_height);
}


static
void parse_command_line(int argc, char *argv[])
{
    int opt;
    static struct option long_options[] = {
        {"display", required_argument, NULL, 'd'},
        {0, 0, 0, 0}  // 结束符
    };

    while ((opt = getopt_long(argc, argv, "d:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                g_xcbscreen_ptr->display_name = optarg;  // 设置 -d 参数的值
                break;
            default:
                print_usage(argc, argv);
                exit(EXIT_FAILURE);
        }
    }
}

static int
init_xcbscreen_info()
{
    const xcb_setup_t *setup;
    int screen_nums = 0;
    xcb_screen_iterator_t iter;

    g_xcbscreen_ptr->connection = xcb_connect(g_xcbscreen_ptr->display_name, NULL);
    if (xcb_connection_has_error(g_xcbscreen_ptr->connection)) {
        fprintf(stderr, "Can not connect to XServer.\n");
        return -1;
    }
    setup = xcb_get_setup(g_xcbscreen_ptr->connection);
    iter = xcb_setup_roots_iterator(setup);
    while (iter.rem) {
        xcb_screen_t *screen;

        // Copy screen info
        g_xcbscreen_ptr->screens[screen_nums].screen = malloc(xcb_screen_sizeof(iter.data));
        memcpy(g_xcbscreen_ptr->screens[screen_nums].screen, iter.data, xcb_screen_sizeof(iter.data));
        screen = g_xcbscreen_ptr->screens[screen_nums].screen;

        // Setup root window
        g_xcbscreen_ptr->screens[screen_nums].root_window = screen->root;

        // Setup screen info
        g_xcbscreen_ptr->screens[screen_nums].window_damage.damage_xpos = 0;
        g_xcbscreen_ptr->screens[screen_nums].window_damage.damage_ypos = 0;
        g_xcbscreen_ptr->screens[screen_nums].window_damage.damage_area_width = screen->width_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].window_damage.damage_area_height= screen->height_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].default_width = screen->width_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].default_height = screen->height_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].scaled_width = screen->width_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].scaled_height = screen->height_in_pixels;
        g_xcbscreen_ptr->screens[screen_nums].scale_factor = 1.0;
        g_xcbscreen_ptr->screens[screen_nums].fbdata = malloc(screen->width_in_pixels * screen->height_in_pixels * 4);
        assert(g_xcbscreen_ptr->screens[screen_nums].fbdata);

        // Setup pixel format
        g_xcbscreen_ptr->screens[screen_nums].screen_pixfmt.pixfmt = detect_screen_pixfmt(screen);

        // Setup root cursor
        g_xcbscreen_ptr->screens[screen_nums].root_cursor.image_data = cursor_arrow_data;
        g_xcbscreen_ptr->screens[screen_nums].root_cursor.mask_data = cursor_arrow_mask;
        g_xcbscreen_ptr->screens[screen_nums].root_cursor.now_shape.shape = CURSOR_SHAPE_ARROW;
        g_xcbscreen_ptr->screens[screen_nums].root_cursor.prev_shape.shape = -1;
        g_xcbscreen_ptr->screen_nums = ++screen_nums;
        xcb_screen_next(&iter);
    }
}

static void
init_xcb_damage_ext()
{
    xcb_query_extension_cookie_t cookie = xcb_query_extension(g_xcbscreen_ptr->connection, strlen("DAMAGE"), "DAMAGE");
	xcb_query_extension_reply_t *reply = xcb_query_extension_reply(g_xcbscreen_ptr->connection, cookie, NULL);
    xcb_generic_error_t *xcb_error;

    g_xcbscreen_ptr->damage_cookie = xcb_damage_query_version(g_xcbscreen_ptr->connection,
        XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);
    g_xcbscreen_ptr->damage_version_reply = xcb_damage_query_version_reply(g_xcbscreen_ptr->connection,
        g_xcbscreen_ptr->damage_cookie, &xcb_error);
    if (!g_xcbscreen_ptr->damage_version_reply) {
        fprintf(stderr, "Initialize DAMAGE extension failed.\n");
        return;
    }
    printf("The DAMAGE extension version(%d,%d)\n", XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);
    g_xcbscreen_ptr->damage_report_level = XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY;
    g_xcbscreen_ptr->damage_id = xcb_generate_id(g_xcbscreen_ptr->connection);
    g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.window_damage_cookie = xcb_damage_create(
        g_xcbscreen_ptr->connection,
        g_xcbscreen_ptr->damage_id,
        g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].root_window,
        g_xcbscreen_ptr->damage_report_level);
    g_xcbscreen_ptr->xcb_event = NULL;
    g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_event = reply->first_event + XCB_DAMAGE_NOTIFY;
    xcb_error = xcb_request_check(g_xcbscreen_ptr->connection,
        g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.window_damage_cookie);
    if (xcb_error) {
        fprintf(stderr, "Failed to create damage object: error code %d\n", xcb_error->error_code);
        return;
    }
}

static void
init_xcb_xfixes_ext()
{
    /* TODO:
     *  Initialize the xfixes extensions, and make sure the XFixes extension is being support.
     */
}

static void
init_xcb_xshm_ext()
{
    /* TODO:
     *  Make sure the X shared memroy is being support.
     */
}

static void
init_xcb_exts()
{
    init_xcb_damage_ext();
    init_xcb_xfixes_ext();
}

static void
init_vncserver_secret()
{
    snprintf(g_secret_store, PATH_MAX, "%s%s.%d", VNCSERVER_SECRET_PATH, "secret", getpid());
    rfbEncryptAndStorePasswd((char *) VNCSERVER_SECRET, (char *) g_secret_store);
}

int
init_vncserver_info(int argc,
                    char *argv[],
                    uint16_t vncport,
                    uint16_t http_vncport)
{
    int screen_nums;
    int i;

    screen_nums = g_xcbscreen_ptr->screen_nums;
    for (i = 0; i < screen_nums; i++) {
        int screen_width, screen_height;
        screen_width = g_xcbscreen_ptr->screens[i].default_width;
        screen_height = g_xcbscreen_ptr->screens[i].default_height;
        g_rfbscreen_ptr[i] = rfbGetScreen(&argc, argv, screen_width, screen_height, 8, 3, 4);
        g_rfbscreen_ptr[i]->frameBuffer = (char *) malloc(screen_width * screen_height * 4);
        if (!(g_rfbscreen_ptr[i]->frameBuffer)) {
            fprintf(stderr, "Initialize the frame buffer failed.\n");
            break;
        }
        g_rfbscreen_ptr[i]->desktopName = DESKTOP_NAME;
        g_rfbscreen_ptr[i]->alwaysShared = TRUE;  // It allows more different client get connection.
        g_rfbscreen_ptr[i]->width = screen_width;
        g_rfbscreen_ptr[i]->height = screen_height;
        g_rfbscreen_ptr[i]->port = VNCSERVER_PORT;
        g_rfbscreen_ptr[i]->newClientHook = get_new_client_connection;
        (*g_rfbscreen_ptr)->authPasswdData = (void*) g_secret_store;
        g_rfbscreen_ptr[i]->httpDir = "./httpvnc";
        g_rfbscreen_ptr[i]->httpPort = http_vncport;
        g_rfbscreen_ptr[i]->httpEnableProxyConnect = TRUE;
        g_rfbscreen_ptr[i]->ptrAddEvent = pointer_event_callback;
        g_rfbscreen_ptr[i]->kbdAddEvent = keyboard_event_callback;
        g_rfbscreen_ptr[i]->setDesktopSizeHook = desktopsize_event_callback;
        g_rfbscreen_ptr[i]->getExtDesktopScreenHook = ext_desktop_screen_event_callback;
        rfbInitServer(g_rfbscreen_ptr[i]);
    }
}

static void
free_vncserver()
{
    for (int i = 0; i < TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS; i++) {
        if (likely(g_rfbscreen_ptr[i] != NULL)) {
            rfbScreenCleanup(g_rfbscreen_ptr[i]);
            g_rfbscreen_ptr[i] = NULL;
        }
    }
}

static void
free_xcbscreen()
{
    for (int i = 0; i < TINYX11_VNCSERVER_SUPPORTED_SCREEN_NUMS; i++) {
        free(g_xcbscreen_ptr->screens[i].screen);
        free(g_xcbscreen_ptr->screens[i].fbdata);
        xcb_disconnect(g_xcbscreen_ptr->connection);
    }
}

int main(int argc, char *argv[])
{
    // uint32_t damage_major_version = 0, damage_minor_version = 0;
    xcb_generic_error_t *error;

    if (argc < 2) {
        print_usage(argc, argv);
        exit(EXIT_FAILURE);
    }
    g_xcbscreen_ptr = malloc(sizeof(*g_xcbscreen_ptr));
    assert(g_xcbscreen_ptr);
    memset(g_xcbscreen_ptr, 0, sizeof(*g_xcbscreen_ptr));

    parse_command_line(argc, argv);
    init_xcbscreen_info();
    init_xcb_exts();
    init_vncserver_secret();
    init_vncserver_info(argc, argv, VNCSERVER_PORT, HTTP_VNCSERVER_PORT);
    rfbRunEventLoop(g_rfbscreen_ptr[DEFAULT_SCREEN_NUM], 10000, TRUE);

    xcb_change_window_attributes(g_xcbscreen_ptr->connection,
                                g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].screen->root,
                                XCB_CW_EVENT_MASK,
                                (const uint32_t[]) {XCB_EVENT_MASK_EXPOSURE});
    xcb_flush(g_xcbscreen_ptr->connection);
    while (TRUE) {
        g_xcbscreen_ptr->xcb_event = xcb_wait_for_event(g_xcbscreen_ptr->connection);
        if (g_xcbscreen_ptr->xcb_event) {
            if ((g_xcbscreen_ptr->xcb_event->response_type & ~0x80)== 
                 g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_event) {
                xcb_damage_notify_event_t *damage_event = (xcb_damage_notify_event_t *)g_xcbscreen_ptr->xcb_event;
                #ifdef DEBUG
                fprintf(stderr, "x, y, width, height: %d, %d, %d, %d\n", damage_event->area.x, damage_event->area.y,
                    damage_event->area.width, damage_event->area.height);
                #endif
                g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_xpos = damage_event->area.x;
                g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_ypos= damage_event->area.y;
                if (damage_event->area.width != 0 && damage_event->area.height != 0) {
                    g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_area_width = damage_event->area.width;
                    g_xcbscreen_ptr->screens[DEFAULT_SCREEN_NUM].window_damage.damage_area_height = damage_event->area.height;
                }
                xcb_xfixes_query_version_cookie_t xfixes_cookie = xcb_xfixes_query_version(
                    g_xcbscreen_ptr->connection, XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION
                );
                xcb_xfixes_query_version_reply_t *xfixes_reply = xcb_xfixes_query_version_reply(
                    g_xcbscreen_ptr->connection,
                    xfixes_cookie,
                    NULL
                );
                
                xcb_damage_subtract(g_xcbscreen_ptr->connection, g_xcbscreen_ptr->damage_id, XCB_NONE, XCB_NONE);
            }
        }
        capture_screen(DEFAULT_SCREEN_NUM);
    }
err:
    free_vncserver();
    free_xcbscreen();
    return 0;
}
