/* 
 * Chip8 main
 * Author: Robert Powell 
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>

#include <GL/glew.h>
#include <GL/glx.h> 
#include <GL/gl.h>
#include <GL/glu.h>

#include <iostream>
#include <string>

#include <cstdio>
#include <cstdlib>

#include <chrono>

#include "util.h"

Display*        display;
XVisualInfo*    vi;
Window          window;
XFontStruct*    font;
GLXFBConfig     fb_config;
GLXContext      gl_ctx;
Colormap        color_map;
int             screen;
uint win_w, win_h;

static int GL_Visual_Attrs[] = {
    GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR, 
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
    None
};

const GLubyte test_bitmap[8*32] = {     0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
                                        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,};   



uint8_t colors[] = { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF};

uint8_t *bg_color = colors, *fg_color = colors+ 3;
uint8_t *bg_red = bg_color, *bg_green = bg_color + 1, *bg_blue = bg_color+2;
uint8_t *fg_red = fg_color, *fg_green = fg_color + 1, *fg_blue = fg_color+2;

float pixel_maps[] = {0.0, 1.0,
                      0.0, 1.0,
                      0.0, 1.0,
                      0.0, 1.0};

float *r_map = pixel_maps, *g_map = pixel_maps + 2, *b_map = pixel_maps + 4, *a_map = pixel_maps + 6;                                                 

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, bool, const int*);

const char* prog_title = "Chip8 Emulator";
const char* font_name = "-misc-hack-medium-r-normal--0-0-0-0-m-0-iso8859-16";

static bool ctxErrorOccurred = false; 
static unsigned char xErrorCode = 0;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    xErrorCode = ev->error_code;
    ctxErrorOccurred = true;
    return 0;
}

static int WaitNotify(Display* d, XEvent* e, XPointer arg){
    return d && e && (e->type = MapNotify) && (e->xmap.window == *(Window*) arg);
}

static int isExtensionSupported(const char *extList, const char *extension)
{
 
  const char *start;
  const char *where, *terminator;
 
  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if ( where || *extension == '\0' )
    return 0;
 
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for ( start = extList; ; ) {
    where = strstr( start, extension );
 
    if ( !where )
      break;
 
    terminator = where + strlen( extension );
 
    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return 1;
 
    start = terminator;
  }
  return 0;
}


static void createWindow(){

    // Open X11 display connection
    D(DBG, "Opening X11 display connection...");
    display = XOpenDisplay(NULL);
    if(!display){
        fatalError(1, "XError: Cannot connect to XServer (%s).\n", XDisplayName(NULL));
    }
    D(SUCC, "Done\n");

    // Check if GLX verison >= 1.3 since fbc was added in 1.3
    int glx_major, glx_minor;
    if(!glXQueryVersion(display, &glx_major, &glx_minor) || (glx_major == 1 && glx_minor < 3)){
        fatalError(1, "Xerror: Requires GLX version 1.3 or greater. Current: %d.%d\n");
    }
    D(DBG, "GLX version: %d.%d\n", glx_major, glx_minor);

    // Get list of matching frame buffer configs
    D(DBG, "Choosing Frame Buffer configuration...");
    int fbc_count;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), GL_Visual_Attrs, &fbc_count);
    fb_config = 0;
    int fb_samples = 0;

    if(!fbc){
        fatalError(1, "No matching frame buffer configurations.\n");
    }

    for(int i=0; i < fbc_count; i++){
        vi = (XVisualInfo*) glXGetVisualFromFBConfig(display, fbc[i]);
		if(!vi)
			continue;

        int samp_buf, samples;
        glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
        glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );
    

        if ( fb_config == 0 || (samp_buf && (samples > fb_samples)))
            fb_config = fbc[i], fb_samples = samples;
        XFree(vi);
    }

    if(!fb_config){
        fatalError(1, "No frame buffer configurations found for specified configuration.\n");
    }

    XFree(fbc);

    vi = glXGetVisualFromFBConfig(display, fb_config);

    #ifdef DEBUG

    int fdb, rb, bb, gb, ab, db;
    glXGetFBConfigAttrib(display, fb_config, GLX_DOUBLEBUFFER, &fdb);
	glXGetFBConfigAttrib(display, fb_config, GLX_RED_SIZE, &rb);
	glXGetFBConfigAttrib(display, fb_config, GLX_GREEN_SIZE, &gb);
	glXGetFBConfigAttrib(display, fb_config, GLX_BLUE_SIZE, &bb);
	glXGetFBConfigAttrib(display, fb_config, GLX_ALPHA_SIZE, &ab);
	glXGetFBConfigAttrib(display, fb_config, GLX_DEPTH_SIZE, &db);
    #endif

    D(SUCC, "Done\n");
    D(DBG, "Chosen FB Configuration:\n\tDouble Buffered: %s\n\tColor Bits:\n\t\tRed: %d\n\t\tGreen: %d\n\t\tBlue: %d\n\t\tAlpha: %d\n\tDepth: %d\n", fdb? "Yes" : "No", rb, gb, bb, ab, db);

    XSetWindowAttributes win_attrs;
    win_attrs.colormap = color_map = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    win_attrs.background_pixmap = None;
	win_attrs.border_pixel = 0;
	win_attrs.event_mask = StructureNotifyMask | KeyPress | KeyRelease;

    int attr_mask = 
		CWColormap|
		CWBorderPixel|
		CWEventMask;

    int width = DisplayWidth(display, screen)/2;
	int height = DisplayHeight(display, screen)/2;
	int y=height/2, x=y*2;

    D(DBG, "Creating X11 window...");

    window = XCreateWindow(display, RootWindow(display, vi->screen), x, y, width, height, 5, vi->depth, InputOutput, vi->visual, attr_mask, &win_attrs);
    if(!window){

        fatalError(1, "Failed to create X11 window.\n");
    }

    XFree(vi);

    D(SUCC, "Done\n");

    XStoreName(display, window, prog_title);

    XSizeHints hints;
	XWMHints *startup_state;
	XTextProperty textprop;

    textprop.value = (unsigned char*) prog_title;
	textprop.encoding = XA_STRING;
	textprop.format = 8;
	textprop.nitems = strlen(prog_title);

	hints.x = x;
	hints.y = y;
	hints.width = width;
	hints.height = height;
	hints.flags = USPosition|USSize;

	startup_state = XAllocWMHints();
	startup_state->initial_state = NormalState;
	startup_state->flags = StateHint;

	XSetWMProperties(display, window, &textprop, &textprop,
			NULL, 0,
			&hints,
			startup_state,
			NULL);

	XFree(startup_state);

    D(DBG, "Mapping window...");

    XMapWindow(display, window);

    XEvent event;

    XIfEvent(display, &event, WaitNotify, (XPointer) &window);

    D(SUCC, "Done\n");

    Atom atom_del = XInternAtom(display, "WM_DELETE_WINDOW", 1);
    XSetWMProtocols(display, window, &atom_del, 1);
}

static void createGLRenderContext(){

    // Query the OpenGL extensions supported by Xserver
    const char* glx_exts = glXQueryExtensionsString(display, DefaultScreen(display));

    // Replace the XServer error handler while attempting to create OpenGL 3.0 context
    gl_ctx = 0;
    ctxErrorOccurred = false;
    xErrorCode = 0;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    D(DBG, "ARB Address: %p\n", glXCreateContextAttribsARB);

    gl_ctx = 0;

    // Use GLX_ARB_Create_Context function if it is supported, otherwise use glx 1.3 context creation
    if(!isExtensionSupported(glx_exts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB){
        D(DBG, "glxCreateAttributesARB() not supported, creating GL context using old-style creation...");
        gl_ctx = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);

        XSync(display, False);
        if(ctxErrorOccurred || !gl_ctx){
            char msgbuff[512];
            XGetErrorText(display, xErrorCode, msgbuff, 512);
            fatalError(1, "Gl Context Creation Failed - XError: %s\n", msgbuff);
        }
        D(SUCC, "Done\n");
    }
    else{
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };


        D(DBG, "Creating GL context using ARB function...");
        gl_ctx = glXCreateContextAttribsARB( display, fb_config, 0,
                                      True, context_attribs );
        XSync(display, False);
        if(!ctxErrorOccurred && gl_ctx)
            D(SUCC, "Done\n");
        else{

            char msgbuff[512];
            XGetErrorText(display, xErrorCode, msgbuff, 512);
            D(ERR, "GL 3.0 context creation failed - XError: %s\n", msgbuff);
            D(DBG, "Creating GL context using old-style creation...");

            ctxErrorOccurred = false;
            xErrorCode = 0;

            context_attribs[1] = 1;
            context_attribs[3] = 0;

            gl_ctx = glXCreateContextAttribsARB( display, fb_config, 0,
                                      True, context_attribs );

            XSync(display, False);
            if(ctxErrorOccurred || !gl_ctx){
                char msgbuff[512];
                XGetErrorText(display, xErrorCode, msgbuff, 512);
                fatalError(1, "Gl context creation failed - XError: %s\n", msgbuff);
            }
            D(SUCC, "Done.\n");

        }
    }

    // Restore XServer Error Handler
    XSetErrorHandler(oldHandler);

    // Verifying that context is a direct context
    if ( ! glXIsDirect ( display, gl_ctx ) )
    {
        D(DBG, "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        D(DBG, "Direct GLX rendering context obtained\n" );
    }

    D(DBG, "Making context current\n" );
    glXMakeCurrent( display, window, gl_ctx );

}

static void updateSurfaceTexture(GLuint tex_id, const GLvoid* tex_dat, uint tex_w, uint tex_h){

    glBindTexture(GL_TEXTURE_2D, tex_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, r_map);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, g_map);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, b_map);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, a_map);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tex_w,tex_h,0,GL_COLOR_INDEX,GL_BITMAP, tex_dat);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

}

static void updateColorMaps(mapper<uint8_t, float> pixel_mapper){
    for(int i=0; i < 3; i++){
        pixel_maps[i * 2] = pixel_mapper.map(colors[i]);
        pixel_maps[i * 2 + 1] = pixel_mapper.map(colors[i+3]);
    }
}

static void drawQuad(GLuint tex_id){

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0, win_h);
    glTexCoord2i(0, 1); glVertex2i(0, 0);
    glTexCoord2i(1, 1); glVertex2i(win_w, 0);
    glTexCoord2i(1, 0); glVertex2i(win_w, win_h);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// static void getKeyMappings(){

// }

static void cleanup(){
    XDestroyWindow( display, window );
    XFreeColormap( display, color_map );
    XCloseDisplay( display );
}

int main(int argc, char** argv){

    createWindow();
    createGLRenderContext();

    GLenum err_code = glewInit();
    if(err_code != GLEW_OK){
        fatalError(1, "GLEW initialization failed. <%d: %s>\n", err_code, glewGetErrorString(err_code));
    }

    Window win_dummy;
    int dummy;
    uint udummy;
    XGetGeometry(display, window, &win_dummy, &dummy, &dummy, &win_w, &win_h, &udummy, &udummy);
    // glViewport(0, 0, win_w, win_h);

    glMatrixMode(GL_PROJECTION);
    glOrtho(0, win_w, 0, win_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    GLuint tex_id;
    glGenTextures(1, &tex_id);

    updateSurfaceTexture(tex_id, test_bitmap, 64, 32);

    drawQuad(tex_id);
    glXSwapBuffers ( display, window );

    XEvent event;

    mapper<uint8_t, float> pixel_mapper(0, 255, 0.f, 1.f);

    //unsigned int key_state = 0;
    //long long timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    D(DEBUG, "FG: R: %d, G: %d, B: %d; BG: R: %d, G: %d, B: %d;", colors[0], colors[1], colors[2], colors[3], colors[4], colors[5]);
    bool running = 1;
    while (running) {
        XNextEvent(display, (XEvent *)&event);
        switch ( event.type ) {
            case Expose:
            {
                XGetGeometry(display, window, &win_dummy, &dummy, &dummy, &win_w, &win_h, &udummy, &udummy);
                break;
            }
            case KeyPress:
                //key_state |= 1 << (event.xkey.keycode - 24);
                //increase color value
                if(event.xkey.keycode >= 24 && event.xkey.keycode <= 29){
                    colors[event.xkey.keycode - 24] = (colors[event.xkey.keycode - 24] == 255)? 255 : colors[event.xkey.keycode - 24] + 1;
                    updateColorMaps(pixel_mapper);
                }
                else if(event.xkey.keycode >= 38 && event.xkey.keycode <= 43){
                    colors[event.xkey.keycode - 38] = (colors[event.xkey.keycode - 38] == 0)? 0 : colors[event.xkey.keycode - 38] - 1;
                    updateColorMaps(pixel_mapper);
                }

                D(DEBUG, "\rFG: R: %*d, G: %*d, B: %*d; BG: R: %*d, G: %*d, B: %*d;", 3, colors[0], 3, colors[1], 3, colors[2], 3, colors[3], 3, colors[4], 3, colors[5]);
                break;
            case KeyRelease:
                //key_state &= 0 << (event.xkey.keycode - 24);
                break;
            case ClientMessage:
                running = 0;
                break;
            default:
                break;
        }

        // long long current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // if(key_state && !timer){
        //     timer = current_time;
        // }
        // else if(key_state && timer){
        //     if(current_time - timer > 500){
        //         unsigned int keys = key_state, index = 0;
        //         while (keys){
        //             if(keys & 0x1){
        //                 if(index < 6 && !(key_state & (1 << (index + 12)))){
        //                     colors[index] = (colors[index] == 255)? colors[index] + 1 : 255;
        //                 }
        //                 if(index >= 12 && index <=18 && !(key_state & (1 << (index - 12)))){
        //                     colors[index-12] = (colors[index - 12] == 0)? colors[index - 12] - 1 : 0;
        //                 }
        //             }
        //         }
        //         updateColorMaps(pixel_mapper);
        //         timer = current_time;
        //     }
        // }
        // else if(!key_state && timer){
        //     timer = 0;
        // }

        // long long current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        // if(current_time - timer > 500){
        //     for(int i=0; i<6; i++){
        //         colors[i] += 1;
        //     }
        //     timer = current_time;
        // }

        updateColorMaps(pixel_mapper);

        updateSurfaceTexture(tex_id, test_bitmap, 64, 32);
        drawQuad(tex_id);
        glXSwapBuffers(display, window);
    }

    cleanup();

    return(0);

}