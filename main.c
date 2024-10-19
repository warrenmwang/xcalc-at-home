#include <X11/X.h>
#include <X11/Xlib.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define POSX 50
#define POXY 50
#define WIDTH 500
#define HEIGHT 500
#define BORDER_WIDTH 1
#define LINE_WIDTH 5
#define SCREEN_NUMBER 0

GC create_gc(Display* dpy, Window window, int line_width)
{
    GC gc;
    XGCValues xgcv;
    unsigned long valuemask;

    xgcv.line_style = LineSolid;
    xgcv.line_width = line_width;
    xgcv.cap_style = CapButt;
    xgcv.join_style = JoinMiter;
    xgcv.fill_style = FillSolid;
    xgcv.foreground = BlackPixel(dpy, SCREEN_NUMBER);
    xgcv.background = WhitePixel(dpy, SCREEN_NUMBER);

    valuemask = GCLineStyle | GCLineWidth | GCCapStyle | GCJoinStyle | GCFillStyle | GCForeground |
                GCBackground;
    gc = XCreateGC(dpy, window, valuemask, &xgcv);

    return gc;
}

void draw_diamond(Display* display, Window window, GC gc, XPoint points[])
{
    XDrawPoints(display, window, gc, points, 4, CoordModeOrigin);
    XDrawLines(display, window, gc, points, 4, CoordModeOrigin);
    XDrawLine(display, window, gc, points[0].x, points[0].y, points[3].x, points[3].y);
}

void draw_things(Display* display, Window window, GC gc, XEvent* ev)
{
    char str[] = "Hello X.org!";
    int strLen = strlen(str);
    XDrawString(display, window, gc, 100, 100, str, strLen);

    int SCALE = 100;

    // Draw a diamond
    XPoint points[] = {
        {1 * SCALE, 2 * SCALE},
        {2 * SCALE, 1 * SCALE},
        {3 * SCALE, 2 * SCALE},
        {2 * SCALE, 3 * SCALE},
    };
    draw_diamond(display, window, gc, points);

    XDrawRectangle(display, window, gc, 150, 150, 30, 30);

    char buff[100];
    snprintf(buff, 100, "XEvent->XAnyEvent->Type: %i", ev->xany.type);
    XDrawString(display, window, gc, 200, 200, buff, strlen(buff));
}

void handle_window_close(Atom wmDeleteMessage, XEvent event, bool* running)
{
    if (event.xclient.data.l[0] == wmDeleteMessage) {
        *running = false;
    }
}

int error_handler(Display* display, XErrorEvent* error)
{
    char buff[256]; // probably good enough for most errors
    XGetErrorText(display, error->error_code, buff, sizeof(buff));
    fprintf(stderr, "X Error: %s\n", buff);
    return 0;
}

int main()
{
    Display* display;
    if (!(display = XOpenDisplay(NULL))) {
        errx(1, "Can't get the display");
    }

    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), POSX, POXY, WIDTH,
                                        HEIGHT, BORDER_WIDTH, BlackPixel(display, SCREEN_NUMBER),
                                        WhitePixel(display, SCREEN_NUMBER));

    XMapWindow(display, window); // map window to screen (shows the created window)
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);

    GC gc = create_gc(display, window, LINE_WIDTH);

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XEvent event;
    bool running = true;
    while (running) {
        XNextEvent(display, &event);

        switch (event.type) {
        case Expose:
            draw_things(display, window, gc, &event);
            break;
        case ButtonPress:
            break;
        case KeyPress:
            break;
        case ClientMessage:
            handle_window_close(wmDeleteMessage, event, &running);
            break;
        default:
            break;
        }
    }

    XSetErrorHandler(error_handler);
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
