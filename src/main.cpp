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

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "renderer.h"
#include "chip8.h"
#include "chip8_util.h"

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

// static void updateSurfaceTexture(GLuint tex_id, const GLvoid* tex_dat, uint tex_w, uint tex_h){

//     glBindTexture(GL_TEXTURE_2D, tex_id);

//     glPixelStorei(GL_UNPACK_ALIGNMENT,1);

//     glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, r_map);
//     glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, g_map);
//     glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, b_map);
//     glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, a_map);

//     glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tex_w,tex_h,0,GL_COLOR_INDEX,GL_BITMAP, tex_dat);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//     glBindTexture(GL_TEXTURE_2D, 0);

// }

// static void updateColorMaps(mapper<uint8_t, float> pixel_mapper){
//     for(int i=0; i < 3; i++){
//         pixel_maps[i * 2] = pixel_mapper.map(colors[i]);
//         pixel_maps[i * 2 + 1] = pixel_mapper.map(colors[i+3]);
//     }
// }

// static void drawQuad(GLuint tex_id){

//     glClear(GL_COLOR_BUFFER_BIT);
//     glBindTexture(GL_TEXTURE_2D, tex_id);
//     glEnable(GL_TEXTURE_2D);
//     glBegin(GL_QUADS);
//     glTexCoord2i(0, 0); glVertex2i(0, win_h);
//     glTexCoord2i(0, 1); glVertex2i(0, 0);
//     glTexCoord2i(1, 1); glVertex2i(win_w, 0);
//     glTexCoord2i(1, 0); glVertex2i(win_w, win_h);
//     glEnd();
//     glDisable(GL_TEXTURE_2D);
//     glBindTexture(GL_TEXTURE_2D, 0);
// }

// static void getKeyMappings(){

// }

uint8_t chip8_memory[TOTAL_MEM_SIZE];

uint8_t* allocate(){
    return chip8_memory;
}

void deallocate(uint8_t* mem){
    return;
}

int main(int argc, char** arv){

    emulator_state_t es;

    initDisplay(es);
    createGLXWindow(es);

    chip8 ch8((uint) 0, (chip8io*) 0);
    ch8.reset();

    int x11_event_fd = ConnectionNumber(es.disp);
    fd_set watch_set;
    timeval timeout = { 0, CLOCK_USEC};

    FD_ZERO(&watch_set);
    FD_SET(x11_event_fd, &watch_set);

    uint8_t running = 1;
    while(running){
        int res = select(x11_event_fd + 1, &watch_set, NULL, NULL, &timeout);
        if(res && res != -1){
            /** Process X11 event **/
            XEvent event;
            XNextEvent(es.disp, &event);
            D(debugMsg("Event: %d\n", event.type));
            switch(event.type){
                case Expose:
                    //Handle expose/window update
                    break;
                case KeyPress:
                    //Handle key press
                    break;
                case KeyRelease:
                    //Handle Key release
                    break;
                case ClientMessage:
                    running = 0;
                    break;
                default:
                    break;
            }
        }
        else{
            /** Select timeout run chip8 tick **/

            /** Reset timeval struct and re-add event queue fd to fd set. **/
            timeout.tv_usec = CLOCK_USEC;
            FD_ZERO(&watch_set);
            FD_SET(x11_event_fd, &watch_set);
        }
    }

    cleanup(es);

    // GLenum err_code = glewInit();
    // if(err_code != GLEW_OK){
    //     fatalError(1, "GLEW initialization failed. <%d: %s>\n", err_code, glewGetErrorString(err_code));
    // }

    // Window win_dummy;
    // int dummy;
    // uint udummy;
    // XGetGeometry(display, window, &win_dummy, &dummy, &dummy, &win_w, &win_h, &udummy, &udummy);
    // // glViewport(0, 0, win_w, win_h);

    // glMatrixMode(GL_PROJECTION);
    // glOrtho(0, win_w, 0, win_h, -1, 1);
    // glMatrixMode(GL_MODELVIEW);

    // GLuint tex_id;
    // glGenTextures(1, &tex_id);

    // updateSurfaceTexture(tex_id, test_bitmap, 64, 32);

    // drawQuad(tex_id);
    // glXSwapBuffers ( display, window );

    // XEvent event;

    // mapper<uint8_t, float> pixel_mapper(0, 255, 0.f, 1.f);

    // //unsigned int key_state = 0;
    // //long long timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // D(DEBUG, "FG: R: %d, G: %d, B: %d; BG: R: %d, G: %d, B: %d;", colors[0], colors[1], colors[2], colors[3], colors[4], colors[5]);
    // bool running = 1;
    // while (running) {
    //     XNextEvent(display, (XEvent *)&event);
    //     switch ( event.type ) {
    //         case Expose:
    //         {
    //             XGetGeometry(display, window, &win_dummy, &dummy, &dummy, &win_w, &win_h, &udummy, &udummy);
    //             break;
    //         }
    //         case KeyPress:
    //             //key_state |= 1 << (event.xkey.keycode - 24);
    //             //increase color value
    //             if(event.xkey.keycode >= 24 && event.xkey.keycode <= 29){
    //                 colors[event.xkey.keycode - 24] = (colors[event.xkey.keycode - 24] == 255)? 255 : colors[event.xkey.keycode - 24] + 1;
    //                 updateColorMaps(pixel_mapper);
    //             }
    //             else if(event.xkey.keycode >= 38 && event.xkey.keycode <= 43){
    //                 colors[event.xkey.keycode - 38] = (colors[event.xkey.keycode - 38] == 0)? 0 : colors[event.xkey.keycode - 38] - 1;
    //                 updateColorMaps(pixel_mapper);
    //             }

    //             D(DEBUG, "\rFG: R: %*d, G: %*d, B: %*d; BG: R: %*d, G: %*d, B: %*d;", 3, colors[0], 3, colors[1], 3, colors[2], 3, colors[3], 3, colors[4], 3, colors[5]);
    //             break;
    //         case KeyRelease:
    //             //key_state &= 0 << (event.xkey.keycode - 24);
    //             break;
    //         case ClientMessage:
    //             running = 0;
    //             break;
    //         default:
    //             break;
    //     }

    //     // long long current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //     // if(key_state && !timer){
    //     //     timer = current_time;
    //     // }
    //     // else if(key_state && timer){
    //     //     if(current_time - timer > 500){
    //     //         unsigned int keys = key_state, index = 0;
    //     //         while (keys){
    //     //             if(keys & 0x1){
    //     //                 if(index < 6 && !(key_state & (1 << (index + 12)))){
    //     //                     colors[index] = (colors[index] == 255)? colors[index] + 1 : 255;
    //     //                 }
    //     //                 if(index >= 12 && index <=18 && !(key_state & (1 << (index - 12)))){
    //     //                     colors[index-12] = (colors[index - 12] == 0)? colors[index - 12] - 1 : 0;
    //     //                 }
    //     //             }
    //     //         }
    //     //         updateColorMaps(pixel_mapper);
    //     //         timer = current_time;
    //     //     }
    //     // }
    //     // else if(!key_state && timer){
    //     //     timer = 0;
    //     // }

    //     // long long current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //     // if(current_time - timer > 500){
    //     //     for(int i=0; i<6; i++){
    //     //         colors[i] += 1;
    //     //     }
    //     //     timer = current_time;
    //     // }

    //     updateColorMaps(pixel_mapper);

    //     updateSurfaceTexture(tex_id, test_bitmap, 64, 32);
    //     drawQuad(tex_id);
    //     glXSwapBuffers(display, window);
    // }

    return(0);

}