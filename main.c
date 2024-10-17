#include <X11/Xlib.h>
#include <string.h>
#include <stdbool.h>

int main()
{
    Display* display = XOpenDisplay(NULL);
    int x = 50;
    int y = 50;
    unsigned int width = 250;
    unsigned int height = 250;
    unsigned int borderWidth = 1;
    unsigned int screenNumber = 0;

    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), x, y, width, height, borderWidth,
                        BlackPixel(display, screenNumber),
                        WhitePixel(display, screenNumber));

    XMapWindow(display, window); // map window to screen (shows the created window)
    XSelectInput(display, window, ExposureMask);

    char str[] = "Hello X.org!";
    int strLen = strlen(str);

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XEvent event;
    bool running = true;
    while (running) {
        XNextEvent(display, &event);

        switch(event.type) {
            case Expose:
                XDrawString(display, window, DefaultGC(display, screenNumber), 100, 100, str, strLen);
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
