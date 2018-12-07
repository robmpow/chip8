#ifndef RENDER_H
#define RENDER_H

#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>

#include <vector>

using namespace std;

extern const char* prog_title;

typedef struct{
    Display* disp;
    Window win;
    GLXContext glx_ctx;
    uint32_t button_state;
}emu_state_t;

void initDisplay(emu_state_t& es);
void createGLXWindow(emu_state_t& es);
void render();
void poll_input();
void cleanup(emu_state_t& es);

#endif