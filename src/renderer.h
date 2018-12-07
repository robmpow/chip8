#ifndef RENDER_H
#define RENDER_H

#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>

#include <vector>

using namespace std;

extern const char* prog_title;

class renderer{

private:
    Display* disp;
    Window win;
    GLXContext glx_ctx;

public:
    renderer();
    void initDisplay();
    void createGLXWindow();
    void render();
    void poll_input();
    ~renderer();
};

#endif