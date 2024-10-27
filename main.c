#include <X11/X.h>
#include <X11/Xlib.h>
#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 0.000000001

#define POSX 50
#define POXY 50
#define WIDTH 500
#define HEIGHT 800
#define BORDER_WIDTH 1
#define LINE_WIDTH 5
#define SCREEN_NUMBER 0
#define NUM_BUTTONS 19
#define SCALE 2.25
#define DISPLAY_BUTTON_INDEX 17

#define STATE_BUILDING_OP1 0
#define STATE_BUILDING_OP2 1
#define STATE_JUST_EVAL 2

typedef struct {
    int id;
    int x, y;
    unsigned int width, height;
    unsigned long border, background, foreground;
    char* label;
} Button;

typedef struct {
    char buf[1000];
    unsigned int p;
} MyBuf;

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

void initialize_buttons(Button buttons[], MyBuf display_button_buf)
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

    create_button(buttons, 15, 0, 100, 100, 50, "Clear");
    create_button(buttons, 18, 100, 100, 50, 50, "Del");
    create_button(buttons, 16, 150, 100, 50, 50, "/");

    display_button_buf.buf[0] = '\0';
    create_button(buttons, DISPLAY_BUTTON_INDEX, 0, 0, 200, 100, display_button_buf.buf);
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

    draw_button(display, window, gc, &buttons[DISPLAY_BUTTON_INDEX], font_struct);
    draw_button(display, window, gc, &buttons[18], font_struct);
}

// Updates the labels of the buttons from our calculator's state.
void update_buttons_from_state(Button buttons[], int* state, char* operator_buf, MyBuf* op1,
                               MyBuf* op2, MyBuf* res, MyBuf* display_button_buf)
{
    Button* display_button = &buttons[DISPLAY_BUTTON_INDEX];
    switch (*state) {
    case 0: // building op 1
        // Just copy the chars from op1 buf in the range [0, p_op1) into the DISPLAY_BUTTON_BUF
        // could be more optimized by reusing the buffer maybe, but I'm too unfamiliar with
        // programming at C level for that kind of thing. Premature optimization is the root of all
        // evil and whatnot. Append a null byte for terminating the string manually.
        strncpy(display_button_buf->buf, op1->buf, op1->p);
        display_button_buf->p = op1->p;
        display_button_buf->buf[display_button_buf->p] = '\0';
        display_button->label = display_button_buf->buf;
        break;
    case 1: // building op 2
        strncpy(display_button_buf->buf, op2->buf, op2->p);
        display_button_buf->p = op2->p;
        display_button_buf->buf[display_button_buf->p] = '\0';
        display_button->label = display_button_buf->buf;
        break;
    case 2: // just evaluated
        // copy res buf contents into display buf contents
        // assuming that res is null terminated AND p_res is on index of the null char.
        for (unsigned int i = 0; i < res->p + 1; i++) {
            display_button_buf->buf[i] = res->buf[i];
        }
        display_button->label = display_button_buf->buf;
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
void handle_clear(MyBuf* op1, MyBuf* op2, MyBuf* res, MyBuf* display_button_buf, int* state,
                  bool* seen_decimal, char* operator_buf)
{
    *state = STATE_BUILDING_OP1;
    *seen_decimal = false;
    *operator_buf = '\0';
    op1->buf[0] = '\0';
    op1->p = 0;
    op2->buf[0] = '\0';
    op2->p = 0;
    res->buf[0] = '\0';
    res->p = 0;
    display_button_buf->buf[0] = '\0';
    display_button_buf->p = 0;
}

void handle_backspace(MyBuf* op1, MyBuf* op2, MyBuf* display_button_buf, int* state,
                      bool* seen_decimal)
{
    switch (*state) {
    case STATE_BUILDING_OP1:
        if (op1->p > 0) {
            --(op1->p);
            if (op1->buf[op1->p] == '.') {
                *seen_decimal = false;
            }
            op1->buf[op1->p] = '\0';
        }
        break;
    case STATE_BUILDING_OP2:
        if (op2->p > 0) {
            --(op2->p);
            if (op2->buf[op2->p] == '.') {
                *seen_decimal = false;
            }
            op2->buf[op2->p] = '\0';
        }
        break;
    case STATE_JUST_EVAL:
        if (op1->p > 0) {
            --(op1->p);
            if (op1->buf[op1->p] == '.') {
                *seen_decimal = false;
            }
            op1->buf[op1->p] = '\0';
        }
        if (display_button_buf->p > 0) {
            --(display_button_buf->p);
            display_button_buf->buf[display_button_buf->p] = '\0';
        }
        break;
    default:
        errx(1, "Got invalid state in handle_backspace");
    }
}

void handle_evaluate(MyBuf* op1, MyBuf* op2, MyBuf* res, int* state, char* operator)
{
    // parse op1_buf and op2_buf
    //   trailing 0's?
    //   negative values (negative signs in front)?
    //   start with the simplest case, regular numbers

    // first terminate the strings
    op1->buf[op1->p] = '\0';
    op2->buf[op2->p] = '\0';
    // then parse them
    float op1_float = (float)atof(op1->buf);
    float op2_float = (float)atof(op2->buf);

    fprintf(stdout, "%f %c %f\n", op1_float, *operator, op2_float);

    // use the operator in operator_buf to compute result and store in res_buf
    float res_num;
    switch (*operator) {
    case '+':
        res_num = op1_float + op2_float;
        break;
    case '-':
        res_num = op1_float - op2_float;
        break;
    case 'x':
        res_num = op1_float * op2_float;
        break;
    case '/':
        // no division by zero
        if (fabs(op2_float - 0) < EPSILON) {
            sprintf(res->buf, "ERR: Division by Zero.");
            res->p = strlen(res->buf);
            return;
        }
        res_num = op1_float / op2_float;
        break;
    default:
        errx(1, "Got invalid operator in handle_evaluate: %c\n", *operator);
    }

    // put the char repr of the float res into both the res and op1 bufs
    // and their ptrs
    sprintf(res->buf, "%.10f", res_num);
    res->p = strlen(res->buf);
    strcpy(op1->buf, res->buf);
    op1->p = res->p;

    // clear op2_buf, reset p_op2
    for (unsigned int i = 0; i < op2->p; i++) {
        op2->buf[i] = '\0';
    }
    op2->p = 0;

    // set state to just evaluated
    *state = STATE_JUST_EVAL;
}

void handle_input_num(char num, int* state, MyBuf* op1, MyBuf* op2)
{
    switch (*state) {
    case STATE_BUILDING_OP1:
        op1->buf[op1->p] = num;
        op1->p++;
        break;
    case STATE_BUILDING_OP2:
        op2->buf[op2->p] = num;
        op2->p++;
        break;
    case STATE_JUST_EVAL:
        errx(1, "TODO: not implemented.");
        break; // TODO:
    default:
        errx(1, "Got an invalid state in handle_input_num.");
    }
}

void handle_input_decimal(int* state, bool* seen_decimal, MyBuf* op1, MyBuf* op2)
{
    if (*seen_decimal) {
        return;
    }

    switch (*state) {
    case STATE_BUILDING_OP1:
        op1->buf[op1->p] = '.';
        op1->p++;
        break;
    case STATE_BUILDING_OP2:
        op2->buf[op2->p] = '.';
        op2->p++;
        break;
    case STATE_JUST_EVAL:
        errx(1, "TODO: not implemented");
        break; // TODO:
    default:
        errx(1, "Got an invalid state in handle_input_decimal.");
    }

    *seen_decimal = true;
}

void handle_input_operator(char operator, int * state, MyBuf* op1, MyBuf* op2, char* operator_buf,
                           bool* seen_decimal, MyBuf* res)
{
    switch (*state) {
    case STATE_BUILDING_OP1:
        // only save operator if had any number(s) inputted for operand 1
        if (op1->p > 0) {
            *seen_decimal = false; // reset seen decimal for operand 2
            *operator_buf = operator;
            *state = STATE_BUILDING_OP2;
        }
        break;
    case STATE_BUILDING_OP2:
        if (op2->p == 0) {
            *operator_buf = operator;
        } else {
            // TODO: call handle evaluate (which should evaluate and save result in op1 and
            // res, AND update state for us)
            // operand 2 already has some values,
            *operator_buf = operator;
            handle_evaluate(op1, op2, res, state, operator_buf);
        }
        break;
    case STATE_JUST_EVAL:
        // Getting an operator input when we just evaluated means that we are taking the
        // the value of the previous calculation's result as the operand 1 with the
        // just inputted operator and are now going to transition into building
        // operator 2.
        *operator_buf = operator;
        *state = STATE_BUILDING_OP2; // previous result should be in res_buf AND op1_buf
    default:
        errx(1, "Got an invalid state in handle_input_operator.");
    }
}

void handle_button_click(Button* button, int* state, bool* seen_decimal, char* operator_buf,
                         MyBuf* op1, MyBuf* op2, MyBuf* res, MyBuf* display_button_buf)
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
        handle_input_num('0', state, op1, op2);
        break;
    case 1: // . (decimal for floating point)
        handle_input_decimal(state, seen_decimal, op1, op2);
        break;
    case 2: // evaluate (=)
        handle_evaluate(op1, op2, res, state, operator_buf);
        break;
    case 3: // 1
        handle_input_num('1', state, op1, op2);
        break;
    case 4: // 2
        handle_input_num('2', state, op1, op2);
        break;
    case 5: // 3
        handle_input_num('3', state, op1, op2);
        break;
    case 6: // add (+)
        handle_input_operator('+', state, op1, op2, operator_buf, seen_decimal, res);
        break;
    case 7: // 4
        handle_input_num('4', state, op1, op2);
        break;
    case 8: // 5
        handle_input_num('5', state, op1, op2);
        break;
    case 9: // 6
        handle_input_num('6', state, op1, op2);
        break;
    case 10: // sub (-)
        handle_input_operator('-', state, op1, op2, operator_buf, seen_decimal, res);
        break;
    case 11: // 7
        handle_input_num('7', state, op1, op2);
        break;
    case 12: // 8
        handle_input_num('8', state, op1, op2);
        break;
    case 13: // 9
        handle_input_num('9', state, op1, op2);
        break;
    case 14: // mul (x)
        handle_input_operator('x', state, op1, op2, operator_buf, seen_decimal, res);
        break;
    case 15: // clear (Clear)
        handle_clear(op1, op2, res, display_button_buf, state, seen_decimal, operator_buf);
        break;
    case 16: // div (/)
        handle_input_operator('/', state, op1, op2, operator_buf, seen_decimal, res);
        break;
    case DISPLAY_BUTTON_INDEX:
        return;
    case 18: // Del
        handle_backspace(op1, op2, display_button_buf, state, seen_decimal);
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

    Button buttons[NUM_BUTTONS];
    MyBuf display_button_buf = {.p = 0};
    initialize_buttons(buttons, display_button_buf);

    bool seen_decimal = false;
    int state = STATE_BUILDING_OP1;
    MyBuf op1 = {.p = 0};
    MyBuf op2 = {.p = 0};
    MyBuf res = {.p = 0};
    char operator_buf = '\0';

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
                                &state, &seen_decimal, &operator_buf, &op1, &op2, &res,
                                &display_button_buf);
            update_buttons_from_state(buttons, &state, &operator_buf, &op1, &op2, &res,
                                      &display_button_buf);
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
