#include <X11/X.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdbool.h>

void drawThings(Display* display, Window window, unsigned int screenNumber)
{
    GC graphicsContext = DefaultGC(display, screenNumber);
    char str[] = "Hello X.org!";
    int strLen = strlen(str);
    XDrawString(display, window, graphicsContext, 100, 100, str, strLen);

    int SCALE = 100;

    // Draw a diamond
    XPoint points[] = {
       {1*SCALE, 2*SCALE},
       {2*SCALE, 1*SCALE},
       {3*SCALE, 2*SCALE},
       {2*SCALE, 3*SCALE},
    };
    XDrawPoints(display, window, graphicsContext, points, 4, CoordModeOrigin);
    XDrawLines(display, window, graphicsContext, points, 4, CoordModeOrigin);
    XDrawLine(display, window, graphicsContext, points[0].x, points[0].y, points[3].x, points[3].y);

    XDrawRectangle(display, window, graphicsContext, 150, 150, 30, 30);
}

int main()
{
    Display* display = XOpenDisplay(NULL);
    int x = 50;
    int y = 50;
    unsigned int width = 500;
    unsigned int height = 500;
    unsigned int borderWidth = 1;
    unsigned int screenNumber = 0;

    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), x, y, width, height, borderWidth,
                        BlackPixel(display, screenNumber),
                        WhitePixel(display, screenNumber));

    XMapWindow(display, window); // map window to screen (shows the created window)
    XSelectInput(display, window, ExposureMask);

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XEvent event;
    bool running = true;
    while (running) {
        XNextEvent(display, &event);

        switch(event.type) {
            case Expose:
                drawThings(display, window, screenNumber);
                break;
            case ClientMessage:
                if (event.xclient.data.l[0] == wmDeleteMessage) {
                    running = false;
                    break;
                }
            default:
                break;
        }
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
