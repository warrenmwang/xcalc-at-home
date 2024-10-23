#include <X11/X.h>
#include <X11/Xlib.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define POSX 50
#define POXY 50
#define WIDTH 500
#define HEIGHT 800
#define BORDER_WIDTH 1
#define LINE_WIDTH 5
#define SCREEN_NUMBER 0
#define NUM_BUTTONS 18
#define SCALE 2.25
#define DISPLAY_BUTTON_INDEX 17

typedef struct {
    int id;
    int x, y;
    unsigned int width, height;
    unsigned long border, background, foreground;
    char* label;
} Button;

Button* get_button_at_location(Button buttons[], int x, int y)
{
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width && y >= buttons[i].y &&
            y <= buttons[i].y + buttons[i].height) {
            return &buttons[i];
        }
    }
    return NULL;
}

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
    btn->id = btn_index;
    btn->x = x * SCALE;
    btn->y = y * SCALE;
    btn->width = w * SCALE;
    btn->height = h * SCALE;
    btn->label = label;
}

void initialize_buttons(Button buttons[], char display_button_buf[])
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

    display_button_buf[0] = '\0';
    create_button(buttons, DISPLAY_BUTTON_INDEX, 0, 0, 200, 100, display_button_buf);
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

// Updates the labels of the buttons from our calculator's state.
void update_buttons_from_state(Button buttons[], int* state, char op1[], unsigned int* p_op1,
                               char op2[], unsigned int* p_op2, char* operator_buf,
                               long long* res_buf, char display_button_buf[],
                               unsigned int* p_display_buf)
{
    Button* display_button = &buttons[DISPLAY_BUTTON_INDEX];
    switch (*state) {
    case 0: // building op 1
        // Just copy the chars from op1 buf in the range [0, p_op1) into the DISPLAY_BUTTON_BUF
        // could be more optimized by reusing the buffer maybe, but I'm too unfamiliar with
        // programming at C level for that kind of thing. Premature optimization is the root of all
        // evil and whatnot. Append a null byte for terminating the string manually.
        strncpy(display_button_buf, op1, *p_op1);
        *p_display_buf = *p_op1;
        display_button_buf[*p_op1] = '\0';
        display_button->label = display_button_buf;
        break;
    case 1: // building op 2
        strncpy(display_button_buf, op2, *p_op2);
        *p_display_buf = *p_op2;
        display_button_buf[*p_op2] = '\0';
        display_button->label = display_button_buf;
        break;
    case 2: // just evaluated
        // Convert the value of res into a char. Could be negative, could be a float, or just an
        // int.
        *p_display_buf = sprintf(display_button_buf, "%lld", *res_buf) + 1; // p pts to null byte
        display_button->label = display_button_buf;
        break;
    default:
        errx(1, "Got an invalid state in update_buttons_from_state");
    }
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

// reset bufs, pointers, and state
// don't need to reset op1 and op2 bufs, just the ptrs, but do need to clear
// the display buf
void handle_clear(int* state, unsigned int* p_op1, unsigned int* p_op2, bool* seen_decimal,
                  char* operator_buf, long long* res_buf, char display_button_buf[],
                  unsigned int* p_display_buf)
{
    *state = 0;
    *p_op1 = 0;
    *p_op2 = 0;
    *seen_decimal = false;
    *res_buf = 0;
    display_button_buf[0] = '\0';
    *p_display_buf = 0;
}

void handle_evaluate(char op1_buf[], char op2_buf[])
{
    // TODO:
    // parse op1_buf and op2_buf
    // use the operator in operator_buf to compute result and store in res_buf
    // copy value of res_buf into op1_buf
    // clear op2_buf and
    // set state to just evaluated
}

void handle_input_num(char num, int* state, char op1[], unsigned int* p_op1, char op2[],
                      unsigned int* p_op2)
{
    switch (*state) {
    case 0: // building op 1
        op1[(*p_op1)++] = num;
        break;
    case 1: // building op 2
        op2[(*p_op2)++] = num;
        break;
    case 2:    // just evaluated
        break; // TODO:
    default:
        errx(1, "Got an invalid state in handle_input_num.");
    }
}

void handle_input_decimal(int* state, bool* seen_decimal, char op1[], unsigned int* p_op1,
                          char op2[], unsigned int* p_op2)
{
    if (*seen_decimal) {
        return;
    }

    switch (*state) {
    case 0: // building op 1
        op1[(*p_op1)++] = '.';
        break;
    case 1: // building op 2
        op2[(*p_op2)++] = '.';
        break;
    case 2:    // just evaluated
        break; // TODO:
    default:
        errx(1, "Got an invalid state in handle_input_decimal.");
    }

    *seen_decimal = true;
}

void handle_input_operator(char operator, int * state, char op1[], unsigned int* p_op1, char op2[],
                           unsigned int* p_op2, char* operator_buf)
{
    switch (*state) {
    case 0: // were building op 1
        // only save operator if had any number(s) inputted for operand 1
        if (*p_op1 > 0) {
            *operator_buf = operator;
            (*state)++;
        }
        break;
    case 1: // were building op 2
        if (*p_op2 == 0) {
            *operator_buf = operator;
        } else {
            // TODO: call handle evaluate (which should evaluate and save result in op1_buf and
            // res_buf, AND update state for us)

            *operator_buf = operator;
        }
        break;
    case 2: // just evaluated
        *operator_buf = operator;
        (*state)--; // previous result should be in res_buf AND op1_buf
    default:
        errx(1, "Got an invalid state in handle_input_operator.");
    }
}

// TODO:
// think about how state should work and how this calculator will work and handle edge cases
// how to get it to display properly and all that as the label of the display button.
void handle_button_click(Button* button, int* state, bool* seen_decimal, char op1[],
                         unsigned int* p_op1, char op2[], unsigned int* p_op2, char* operator_buf,
                         long long* res_buf, char display_button_buff[],
                         unsigned int* p_display_buf)
{
    if (button == NULL) {
        return;
    }
    if (button->id == DISPLAY_BUTTON_INDEX) {
        return;
    }
    fprintf(stdout, "user clicked on the button with label: %s\n", button->label);

    switch (button->id) {
    case 0: // 0
        handle_input_num('0', state, op1, p_op1, op2, p_op2);
        break;
    case 1: // . (decimal for floating point)
        handle_input_decimal(state, seen_decimal, op1, p_op1, op2, p_op2);
        break;
    case 2: // evaluate (=)
        handle_evaluate(op1, op2);
        break;
    case 3: // 1
        handle_input_num('1', state, op1, p_op1, op2, p_op2);
        break;
    case 4: // 2
        handle_input_num('2', state, op1, p_op1, op2, p_op2);
        break;
    case 5: // 3
        handle_input_num('3', state, op1, p_op1, op2, p_op2);
        break;
    case 6: // add (+)
        handle_input_operator('+', state, op1, p_op1, op2, p_op2, operator_buf);
        break;
    case 7: // 4
        handle_input_num('4', state, op1, p_op1, op2, p_op2);
        break;
    case 8: // 5
        handle_input_num('5', state, op1, p_op1, op2, p_op2);
        break;
    case 9: // 6
        handle_input_num('6', state, op1, p_op1, op2, p_op2);
        break;
    case 10: // sub (-)
        handle_input_operator('-', state, op1, p_op1, op2, p_op2, operator_buf);
        break;
    case 11: // 7
        handle_input_num('7', state, op1, p_op1, op2, p_op2);
        break;
    case 12: // 8
        handle_input_num('8', state, op1, p_op1, op2, p_op2);
        break;
    case 13: // 9
        handle_input_num('9', state, op1, p_op1, op2, p_op2);
        break;
    case 14: // mul (x)
        handle_input_operator('x', state, op1, p_op1, op2, p_op2, operator_buf);
        break;
    case 15: // clear (Clear)
        handle_clear(state, p_op1, p_op2, seen_decimal, operator_buf, res_buf, display_button_buff,
                     p_display_buf);
        break;
    case 16: // div (/)
        handle_input_operator('/', state, op1, p_op1, op2, p_op2, operator_buf);
        break;
    case DISPLAY_BUTTON_INDEX:
        return;
    default:
        break;
    }
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
    XStoreName(display, window, "xcalc at home");

    XMapWindow(display, window); // map window to screen (shows the created window)
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);

    Font font = XLoadFont(display, "10x20");
    if (font == None) {
        errx(1, "Could not load font");
    }

    GC gc = create_gc(display, window, LINE_WIDTH, font);

    XFontStruct* font_struct = XQueryFont(display, XGContextFromGC(gc));
    if (font_struct == NULL) {
        errx(1, "couldn't query font struct");
    }

    Atom wm_delete_message = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_message, 1);

    char display_button_buf[1000];
    unsigned int p_display_buf = 0;

    Button buttons[NUM_BUTTONS];
    bool seen_decimal = false;
    int state = 0; // 0 = building op1, 1 = building op2, 2 = just evaluated
    char op1[1000];
    unsigned int p_op1 = 0;
    char op2[1000];
    unsigned int p_op2 = 0;
    char operator_buf = '\0';
    long long res_buf = 0;
    initialize_buttons(buttons, display_button_buf);

    XEvent event;
    bool running = true;
    while (running) {
        XNextEvent(display, &event);

        switch (event.type) {
        case Expose:
            draw_app(display, window, gc, &event, buttons, font_struct);
            break;
        case ButtonPress:
            handle_button_click(get_button_at_location(buttons, event.xbutton.x, event.xbutton.y),
                                &state, &seen_decimal, op1, &p_op1, op2, &p_op2, &operator_buf,
                                &res_buf, display_button_buf, &p_display_buf);
            update_buttons_from_state(buttons, &state, op1, &p_op1, op2, &p_op2, &operator_buf,
                                      &res_buf, display_button_buf, &p_display_buf);
            XClearWindow(display, window);
            draw_app(display, window, gc, &event, buttons, font_struct);
            break;
        case KeyPress:
            break;
        case ClientMessage:
            handle_window_close(wm_delete_message, event, &running);
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
