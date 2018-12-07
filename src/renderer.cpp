#include "renderer.h"
#include "chip8_util.h"

#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>

#include <GL/glew.h>
#include <GL/glx.h> 
#include <GL/gl.h>
#include <GL/glu.h>

const char* prog_title = "Chip8 Emu";

renderer::renderer(){

}

void renderer::initDisplay(){
    // Open X11 display connection
    D(debugMsg("Opening X11 Display connection..."));
    disp = XOpenDisplay(NULL);
    if(!disp){
        fatalError(1, "XError: Cannot connect to XServer (%s).\n", XDisplayName(NULL));
    }
    D(okMsg("Done\n"));

    // Check if GLX verison >= 1.3 since fbc was added in 1.3
    int glx_major, glx_minor;
    if(!glXQueryVersion(disp, &glx_major, &glx_minor) || (glx_major == 1 && glx_minor < 3)){
        fatalError(1, "Xerror: Requires GLX version 1.3 or greater. Current: %d.%d\n");
    }
    D(debugMsg("GLX version: %d.%d\n", glx_major, glx_minor));
}


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

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, bool, const int*);

static bool ctxErrorOccurred = false; 
static unsigned char xErrorCode = 0;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    xErrorCode = ev->error_code;
    ctxErrorOccurred = true;
    return 0;
}

void renderer::createGLXWindow(){
    // Get list of matching frame buffer configs
    D(debugMsg("Choosing frame buffer configuration..."));
    int fbc_count;
    GLXFBConfig* fbc = glXChooseFBConfig(disp, DefaultScreen(disp), GL_Visual_Attrs, &fbc_count);
    GLXFBConfig fb_config = 0;
    int fb_samples = 0;

    if(!fbc){
        fatalError(1, "No matching frame buffer configurations.\n");
    }

    XVisualInfo* vi;
    for(int i=0; i < fbc_count; i++){
        vi = (XVisualInfo*) glXGetVisualFromFBConfig(disp, fbc[i]);
		if(!vi)
			continue;

        int samp_buf, samples;
        glXGetFBConfigAttrib( disp, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
        glXGetFBConfigAttrib( disp, fbc[i], GLX_SAMPLES       , &samples  );
    

        if ( fb_config == 0 || (samp_buf && (samples > fb_samples)))
            fb_config = fbc[i], fb_samples = samples;
        XFree(vi);
    }

    if(!fb_config){
        fatalError(1, "No frame buffer configurations found for specified configuration.\n");
    }

    XFree(fbc);

    vi = glXGetVisualFromFBConfig(disp, fb_config);

    #ifdef DEBUG

    int fdb, rb, bb, gb, ab, db;
    glXGetFBConfigAttrib(disp, fb_config, GLX_DOUBLEBUFFER, &fdb);
	glXGetFBConfigAttrib(disp, fb_config, GLX_RED_SIZE, &rb);
	glXGetFBConfigAttrib(disp, fb_config, GLX_GREEN_SIZE, &gb);
	glXGetFBConfigAttrib(disp, fb_config, GLX_BLUE_SIZE, &bb);
	glXGetFBConfigAttrib(disp, fb_config, GLX_ALPHA_SIZE, &ab);
	glXGetFBConfigAttrib(disp, fb_config, GLX_DEPTH_SIZE, &db);
    #endif

    D(okMsg("Done\n"));
    D(debugMsg("Chosen FB Configuration:\n\tDouble Buffered: %s\n\tColor Bits:\n\t\tRed: %d\n\t\tGreen: %d\n\t\tBlue: %d\n\t\tAlpha: %d\n\tDepth: %d\n", fdb? "Yes" : "No", rb, gb, bb, ab, db));

    XSetWindowAttributes win_attrs;
    win_attrs.colormap = XCreateColormap(disp, RootWindow(disp, vi->screen), vi->visual, AllocNone);
    win_attrs.background_pixmap = None;
	win_attrs.border_pixel = 0;
	win_attrs.event_mask = StructureNotifyMask | KeyPress | KeyRelease;

    int attr_mask = 
		CWColormap|
		CWBorderPixel|
		CWEventMask;

    int width = DisplayWidth(disp, vi->screen/2);
	int height = DisplayHeight(disp, vi->screen/2);
	int y=height/2, x=y*2;

    D(debugMsg("Creating X11 window..."));

    win = XCreateWindow(disp, RootWindow(disp, vi->screen), x, y, width, height, 5, vi->depth, InputOutput, vi->visual, attr_mask, &win_attrs);
    if(!win){

        fatalError(1, "Failed to create X11 window.\n");
    }

    XFree(vi);

    D(debugMsg("Done.\n"));

    XStoreName(disp, win, prog_title);

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

	XSetWMProperties(disp, win, &textprop, &textprop,
			NULL, 0,
			&hints,
			startup_state,
			NULL);

	XFree(startup_state);

    D(debugMsg("Mapping window..."));

    XMapWindow(disp, win);

    XEvent event;

    XIfEvent(disp, &event, WaitNotify, (XPointer) &win);

    D(okMsg("Done\n"));

    Atom atom_del = XInternAtom(disp, "WM_DELETE_WINDOW", 1);
    XSetWMProtocols(disp, win, &atom_del, 1);

    // Query the OpenGL extensions supported by Xserver
    const char* glx_exts = glXQueryExtensionsString(disp, DefaultScreen(disp));

    // Replace the XServer error handler while attempting to create OpenGL 3.0 context
    ctxErrorOccurred = false;
    xErrorCode = 0;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
            glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

    D(debugMsg("ARB Address: %p\n", glXCreateContextAttribsARB));

    glx_ctx = 0;

    // Use GLX_ARB_Create_Context function if it is supported, otherwise use glx 1.3 context creation
    if(!isExtensionSupported(glx_exts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB){
        D(debugMsg("glxCreateAttributesARB() not supported, creating GL context using old-style creation..."));
        glx_ctx = glXCreateNewContext(disp, fb_config, GLX_RGBA_TYPE, 0, True);

        XSync(disp, False);
        if(ctxErrorOccurred || !glx_ctx){
            char msgbuff[512];
            XGetErrorText(disp, xErrorCode, msgbuff, 512);
            fatalError(1, "Gl Context Creation Failed - XError: %s\n", msgbuff);
        }
        D(okMsg("Done\n"));
    }
    else{
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 0,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };


        D(debugMsg("Creating GL context using ARB function..."));
        glx_ctx = glXCreateContextAttribsARB( disp, fb_config, 0,
                                      True, context_attribs );
        XSync(disp, False);
        if(!ctxErrorOccurred && glx_ctx)
            D(okMsg("Done\n"));
        else{

            char msgbuff[512];
            XGetErrorText(disp, xErrorCode, msgbuff, 512);
            D(errorMsg("GL 3.0 context creation failed - XError: %s\n", msgbuff));
            D(debugMsg("Creating GL context using old-style creation..."));

            ctxErrorOccurred = false;
            xErrorCode = 0;

            context_attribs[1] = 1;
            context_attribs[3] = 0;

            glx_ctx = glXCreateContextAttribsARB( disp, fb_config, 0,
                                      True, context_attribs );

            XSync(disp, False);
            if(ctxErrorOccurred || !glx_ctx){
                char msgbuff[512];
                XGetErrorText(disp, xErrorCode, msgbuff, 512);
                fatalError(1, "Gl context creation failed - XError: %s\n", msgbuff);
            }
            D(okMsg("Done\n"));

        }
    }

    // Restore XServer Error Handler
    XSetErrorHandler(oldHandler);

    // Verifying that context is a direct context
    if ( ! glXIsDirect ( disp, glx_ctx ) )
    {
        D(debugMsg("Indirect GLX rendering context obtained\n" ));
    }
    else
    {
        D(debugMsg("Direct GLX rendering context obtained\n" ));
    }

    D(debugMsg("Making context current\n"));
    glXMakeCurrent( disp, win, glx_ctx );
}

renderer::~renderer(){
    if(disp){
        XCloseDisplay(disp);
    }
}

void renderer::render(){

}

void renderer::poll_input(){

}
