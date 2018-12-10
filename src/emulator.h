#ifndef EMULATOR_H
#define EMULATOR_H

#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>

#include <vector>

using namespace std;

extern const char* prog_title;

typedef struct{
    uint16_t width;
    uint16_t height;
} res_t;

class emulator{
private:
    res_t res;
    Display* disp;
    Window win;
    GLXContext glx_ctx;
    uint32_t button_state;

 public:
    emulator();
    Display* getDisplay();
    Window getWindow();
    GLXContext getGLXCtx();
    void setAllButtons(uint32_t new_val);
    uint32_t getAllButtons();
    void setButton(char button, uint8_t new_val);
    uint8_t getButton(char button);
    void createGLXWindow();
    void setRes(res_t res);
    void setRes(uint16_t width, uint16_t height);
    res_t getRes();
    ~emulator();
};

#endif