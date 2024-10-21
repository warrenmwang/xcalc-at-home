#include <X11/X.h>
#include <X11/Xlib.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define POSX 50
#define POXY 50
#define WIDTH 600
#define HEIGHT 800
#define BORDER_WIDTH 1
#define LINE_WIDTH 5
#define SCREEN_NUMBER 0
#define NUM_BUTTONS 18
#define SCALE 2.25

typedef struct {
    int x, y;
    unsigned int width, height;
    unsigned long border, background, foreground;
    char* label;
} Button;

GC create_gc(Display* display, Window window, int line_width, Font font)
{
    GC gc;
    XGCValues xgcv;
    unsigned long valuemask;

    xgcv.font = font;
    xgcv.line_style = LineSolid;
    xgcv.line_width = line_width;
    xgcv.cap_style = CapButt;
    xgcv.join_style = JoinMiter;
    xgcv.fill_style = FillSolid;
    xgcv.foreground = BlackPixel(display, SCREEN_NUMBER);
    xgcv.background = WhitePixel(display, SCREEN_NUMBER);

    valuemask = GCFont | GCLineStyle | GCLineWidth | GCCapStyle | GCJoinStyle | GCFillStyle |
                GCForeground | GCBackground;
    gc = XCreateGC(display, window, valuemask, &xgcv);

    return gc;
}

void create_button(Button buttons[], int btn_index, int x, int y, int w, int h, char* label)
{
    if (btn_index < 0 || btn_index > NUM_BUTTONS - 1) {
        return;
    }
    Button* btn = &buttons[btn_index];
    btn->x = x * SCALE;
    btn->y = y * SCALE;
    btn->width = w * SCALE;
    btn->height = h * SCALE;
    btn->label = label;
}

void draw_button(Display* display, Window window, GC gc, Button* button, XFontStruct* font_struct)
{
    XDrawRectangle(display, window, gc, button->x, button->y, button->width, button->height);
    int text_width = XTextWidth(font_struct, button->label, strlen(button->label));
    int text_height = 5;

    XDrawString(display, window, gc, button->x + (button->width - text_width) / 2,
                button->y + (button->height - text_height) / 2, button->label,
                strlen(button->label));
}

void draw_app(Display* display, Window window, GC gc, XEvent* ev, Button buttons[],
              XFontStruct* font_struct)
{
    // Create our buttons
    create_button(buttons, 0, 0, 300, 100, 50, "0");
    create_button(buttons, 1, 100, 300, 50, 50, ".");
    create_button(buttons, 2, 150, 300, 50, 50, "=");

    create_button(buttons, 3, 0, 250, 50, 50, "1");
    create_button(buttons, 4, 50, 250, 50, 50, "2");
    create_button(buttons, 5, 100, 250, 50, 50, "3");
    create_button(buttons, 6, 150, 250, 50, 50, "+");

    create_button(buttons, 7, 0, 200, 50, 50, "4");
    create_button(buttons, 8, 50, 200, 50, 50, "5");
    create_button(buttons, 9, 100, 200, 50, 50, "6");
    create_button(buttons, 10, 150, 200, 50, 50, "-");

    create_button(buttons, 11, 0, 150, 50, 50, "7");
    create_button(buttons, 12, 50, 150, 50, 50, "8");
    create_button(buttons, 13, 100, 150, 50, 50, "9");
    create_button(buttons, 14, 150, 150, 50, 50, "x");

    create_button(buttons, 15, 0, 100, 150, 50, "Clear");
    create_button(buttons, 16, 150, 100, 50, 50, "/");

    create_button(buttons, 17, 0, 0, 200, 100, "Hello, this is your X calculator speaking.");

    // Draw our buttons
    draw_button(display, window, gc, &buttons[0], font_struct);
    draw_button(display, window, gc, &buttons[1], font_struct);
    draw_button(display, window, gc, &buttons[2], font_struct);

    draw_button(display, window, gc, &buttons[3], font_struct);
    draw_button(display, window, gc, &buttons[4], font_struct);
    draw_button(display, window, gc, &buttons[5], font_struct);
    draw_button(display, window, gc, &buttons[6], font_struct);

    draw_button(display, window, gc, &buttons[7], font_struct);
    draw_button(display, window, gc, &buttons[8], font_struct);
    draw_button(display, window, gc, &buttons[9], font_struct);
    draw_button(display, window, gc, &buttons[10], font_struct);

    draw_button(display, window, gc, &buttons[11], font_struct);
    draw_button(display, window, gc, &buttons[12], font_struct);
    draw_button(display, window, gc, &buttons[13], font_struct);
    draw_button(display, window, gc, &buttons[14], font_struct);

    draw_button(display, window, gc, &buttons[15], font_struct);
    draw_button(display, window, gc, &buttons[16], font_struct);

    draw_button(display, window, gc, &buttons[17], font_struct);
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

    Font font = XLoadFont(display, "fixed");
    if (font == None) {
        errx(1, "Could not load font: %s\n", "fixed");
    }

    GC gc = create_gc(display, window, LINE_WIDTH, font);

    XFontStruct* font_struct = XQueryFont(display, XGContextFromGC(gc));
    if (font_struct == NULL) {
        errx(1, "couldn't query font struct");
    }

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    Button buttons[NUM_BUTTONS];

    XEvent event;
    bool running = true;
    while (running) {
        XNextEvent(display, &event);

        switch (event.type) {
        case Expose:
            draw_app(display, window, gc, &event, buttons, font_struct);
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
    XFreeFont(display, font_struct);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
