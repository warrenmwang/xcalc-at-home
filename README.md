# Dumber `xcalc`

This is a simple calculator app drawn using the X client lib for X Server protocol.

## Motivation
I felt like I haven't touched C in a long time, and there's no better way to make
reviewing something old by also learning something new at the same time...
I don't know any GUI frameworks for native apps on any of the big three OS's frankly, but I know that 
my system uses X11 for its windowing system and you could probably just use a programming lib
for it directly...How hard could it be?

> What did I get myself into? X11 is dying in favor of things like Wayland and Hyprland.
> Who cares, build the thing! Learn!

## Getting Started

Have an X11 server running. Compile and run program:
```
make && ./main.out
```

## Resources
- https://www.youtube.com/watch?v=d2E7ryHCK08
- https://stackoverflow.com/questions/10792361/how-do-i-gracefully-exit-an-x11-event-loop
- https://tronche.com/gui/x/xlib
- https://xopendisplay.hilltopia.ca/2009/Mar/Xlib-tutorial-part-9----Buttons.html
- Claude 3.5 Sonnet 
