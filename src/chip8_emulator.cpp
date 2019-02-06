#include <iostream>
#include <iomanip>
#include <SDL2/SDL_endian.h>
#include <chrono>
#include <unistd.h>
#include <functional>

#include "chip8_emulator.h"
#include "chip8_util.h"

template<uint32_t>
uint32_t mapColorFormat(SDL_Color& color);

template<uint32_t>
uint32_t mapColorFormat(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

template<>
uint32_t mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return  ((r <<  0) & 0x000000FF) |
                ((g <<  8) & 0x0000FF00) |
                ((b << 16) & 0x00FF0000) |
                ((a << 24) & 0xFF000000);
    #else
        return  ((r << 24) & 0xFF000000) |
            ((g << 16) & 0x00FF0000) |
            ((b <<  8) & 0x0000FF00) |
            ((a <<  0) & 0x000000FF);
    #endif
}

template<>
uint32_t mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(SDL_Color& color){
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return  ((color.r <<  0) & 0x000000FF) |
                ((color.g <<  8) & 0x0000FF00) |
                ((color.b << 16) & 0x00FF0000) |
                ((color.a << 24) & 0xFF000000);
    #else
        return  ((color.r << 24) & 0xFF000000) |
                ((color.g << 16) & 0x00FF0000) |
                ((color.b <<  8) & 0x0000FF00) |
                ((color.a <<  0) & 0x000000FF);
    #endif
}

namespace arg = std::placeholders;

#ifdef NO_SEED
    chip8_emulator::chip8_emulator(std::pair<int, int>& resolution, std::pair<SDL_Color, SDL_Color>& palette, std::string& rom_path, std::unordered_map<key_enum, key_action> binds) : running(true), chip8_running(true), chip8_paused(false), rom_path(rom_path), chip8_instance(0, rom_path), input_handler(binds),{
#else
    chip8_emulator::chip8_emulator(std::pair<int, int>& resolution, std::pair<SDL_Color, SDL_Color>& palette, std::string& rom_path, std::unordered_map<key_enum, key_action> binds) : running(true), chip8_running(true), chip8_paused(false), rom_path(rom_path), chip8_instance(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), rom_path), input_handler(binds){
#endif

    this->window = SDL_CreateWindow("Chip8 Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resolution.first, resolution.second, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    this->renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    int render_target_w, render_target_h;
    SDL_GetRendererOutputSize(renderer, &render_target_w, &render_target_h);

    window_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, render_target_w, render_target_h);

    frame_tex_fg_color =    mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(palette.first);

    frame_tex_bg_color =    mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(palette.second);

    //chip8_logger.log<logger::log_info>("Frame Foreground color: 0x", std::hex, std::setw(8), std::setfill('0'), frame_tex_fg_color, "; Background color: ", std::hex, std::setw(8), std::setfill('0'), frame_tex_bg_color, "\n");

    frame_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    int bpp;
    uint32_t r_mask, g_mask, b_mask, a_mask;
    SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &r_mask, &g_mask, &b_mask, &a_mask);

    SDL_Surface* pause_surface_bmp = SDL_LoadBMP("./res/ch8_paused.bmp");
    SDL_Surface* pause_surface = SDL_ConvertSurfaceFormat(pause_surface_bmp, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(pause_surface_bmp);

    uint32_t pause_bmp_fg_color = SDL_MapRGBA(pause_surface->format, 0xFF, 0xFF, 0xFF, 0xFF);
    uint32_t pause_bmp_bg_color = SDL_MapRGBA(pause_surface->format, 0x00, 0x00, 0x00, 0xFF);

    //chip8_logger.log<logger::log_info>("Frame Foreground color: 0x", std::hex, std::setw(8), std::setfill('0'), pause_bmp_fg_color, "; Background color: ", std::hex, std::setw(8), std::setfill('0'), pause_bmp_bg_color, "\n");


    if(frame_tex_fg_color != pause_bmp_fg_color || frame_tex_bg_color != pause_bmp_bg_color){
        if(!SDL_LockSurface(pause_surface)){
            uint32_t* pixels = static_cast<uint32_t*>(pause_surface->pixels);
            for(int j = 0; j < pause_surface->h; j++){
                for(int i = 0; i < pause_surface->w; i++){
                    if(pixels[i + j * (pause_surface->pitch / sizeof(uint32_t))] == pause_bmp_fg_color){
                        pixels[i + j * (pause_surface->pitch / sizeof(uint32_t))] = frame_tex_fg_color;
                    }
                    if(pixels[i + j * (pause_surface->pitch / sizeof(uint32_t))] == pause_bmp_bg_color){
                        pixels[i + j * (pause_surface->pitch / sizeof(uint32_t))] = frame_tex_bg_color;
                    }
                }
            }
            SDL_UnlockSurface(pause_surface);
        }
    }

    pause_tex = SDL_CreateTextureFromSurface(renderer, pause_surface);

    SDL_FreeSurface(pause_surface);

    int pause_tex_w, pause_tex_h;
    SDL_QueryTexture(pause_tex, NULL, NULL, &pause_tex_w, &pause_tex_h);

    if(render_target_w / 2 > pause_tex_w && render_target_h > pause_tex_h){
        pause_render_boundary.w = render_target_w / 2;
        pause_render_boundary.h = (pause_render_boundary.w * pause_tex_h) / pause_tex_w;
        pause_render_boundary.x = (render_target_w / 4);
        pause_render_boundary.y = (render_target_h - pause_render_boundary.h) / 2;
    }
    else{
        pause_render_boundary.w = pause_tex_w;
        pause_render_boundary.h = pause_tex_h;
        pause_render_boundary.x = 0;
        pause_render_boundary.y = 0;
    }

    input_handler.bind_action(KEY_CH8_0, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_0));
    input_handler.bind_action(KEY_CH8_1, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_1));
    input_handler.bind_action(KEY_CH8_2, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_2));
    input_handler.bind_action(KEY_CH8_3, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_3));
    input_handler.bind_action(KEY_CH8_4, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_4));
    input_handler.bind_action(KEY_CH8_5, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_5));
    input_handler.bind_action(KEY_CH8_6, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_6));
    input_handler.bind_action(KEY_CH8_7, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_7));
    input_handler.bind_action(KEY_CH8_8, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_8));
    input_handler.bind_action(KEY_CH8_9, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_9));
    input_handler.bind_action(KEY_CH8_A, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_A));
    input_handler.bind_action(KEY_CH8_B, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_B));
    input_handler.bind_action(KEY_CH8_C, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_C));
    input_handler.bind_action(KEY_CH8_D, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_D));
    input_handler.bind_action(KEY_CH8_E, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_E));
    input_handler.bind_action(KEY_CH8_F, std::bind(&chip8::update_keystate, &chip8_instance, arg::_1, arg::_2, KEY_F));
    input_handler.bind_action(KEY_EMU_RESET, std::bind(&chip8_emulator::handle_reset, this, arg::_1, arg::_2));
    input_handler.bind_action(KEY_EMU_PAUSE, std::bind(&chip8_emulator::handle_pause, this, arg::_1, arg::_2));
}

void chip8_emulator::handle_pause(bool press_state, bool repeat){

    if(press_state && !repeat){
        chip8_paused = !chip8_paused;
    }

    if(chip8_paused){
        render_pause(true);
    }
    else{
        render_frame();
    }
}

void chip8_emulator::handle_reset(bool press_state, bool repeat){
    if(press_state && !repeat){
        chip8_instance.reset();
        chip8_instance.load(rom_path);
    }
}

void chip8_emulator::run(){

    SDL_Event e;

    std::chrono::high_resolution_clock::time_point beg, end;
    while(running){
        beg = std::chrono::high_resolution_clock::now();
        if(running && chip8_running && !chip8_paused){
            try{
                tick_result res = chip8_instance.run_tick();
                if(res.display_update){
                    render_frame();
                }
                update_sound_state(res.sound_state);
            }catch(std::string error_msg){
                std::cout << error_msg << std::endl;
                chip8_running = NOT_RUNNING;
            }
        }
        else if(chip8_paused){
            render_pause(false);
        }
        while(running && SDL_PollEvent(&e)){
            switch(e.type){
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    input_handler.dispatch(e.key.keysym.sym, e.type == SDL_KEYDOWN, e.key.repeat);
                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event){
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        case SDL_WINDOWEVENT_RESIZED:
                            update_target_size();
                            break;
                        case SDL_WINDOWEVENT_EXPOSED:
                            if(chip8_paused){
                                render_pause(true);
                            }
                            else{
                                render_frame();
                            }
                    }
                    break;
            }
        }
        end = std::chrono::high_resolution_clock::now();
        ticks++;
        auto sleep_duration = CHIP8_TICK_PERIOD_USEC - std::chrono::duration_cast<std::chrono::microseconds>(end - beg).count();
        if(sleep_duration > 0){
            usleep(sleep_duration);
        }
    }
}

void chip8_emulator::render_frame(){
    uint32_t* frame_tex_pixels = 0;
    int frame_tex_pitch = 0;

    if(!SDL_LockTexture(frame_tex, NULL, (void**)&frame_tex_pixels, &frame_tex_pitch)){
        uint8_t* ch8_display = chip8_instance.getDisplay();
        for(uint ind = 0; ind < DISP_SIZE; ind++){
            uint8_t disp_byte = ch8_display[ind];
            for(uint bit_ind = 0; bit_ind < BIT_WIDTH(uint8_t); bit_ind++){
                frame_tex_pixels[ind * BIT_WIDTH(uint8_t) + (7 - bit_ind)] = (disp_byte & 0x01)? frame_tex_fg_color : frame_tex_bg_color;
                disp_byte >>= 1;
            }
        }

    }
    frame_tex_pixels = 0;
    SDL_UnlockTexture(frame_tex);

    SDL_BlendMode renderer_blendmode;
    SDL_GetRenderDrawBlendMode(renderer, &renderer_blendmode);

    SDL_RenderCopy(renderer, frame_tex, NULL, NULL);
    SDL_RenderPresent(renderer);

}

void chip8_emulator::render_pause(bool force_update){

    static long last_blink_ticks = ticks;
    static bool show_message = true;

    if(force_update){
        last_blink_ticks = ticks;
        show_message = true;
    }

    if((ticks - last_blink_ticks > PAUSE_BLINK_INTERVAL) || force_update){
        SDL_SetRenderTarget(renderer, window_tex);

        SDL_RenderClear(renderer);

        SDL_SetTextureAlphaMod(frame_tex, 0x7F);
        SDL_SetTextureBlendMode(frame_tex, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, frame_tex, NULL, NULL);
        SDL_SetTextureAlphaMod(frame_tex, 0xFF);
        SDL_SetTextureBlendMode(frame_tex, SDL_BLENDMODE_BLEND);

        if(show_message)
            SDL_RenderCopy(renderer, pause_tex, NULL, &pause_render_boundary);

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, window_tex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    if(ticks - last_blink_ticks > PAUSE_BLINK_INTERVAL){
        show_message = !show_message;
        last_blink_ticks = ticks;
    }
}

void chip8_emulator::update_sound_state(bool state){
    return;
}

void chip8_emulator::update_target_size(){
    SDL_DestroyTexture(window_tex);

    int render_target_w, render_target_h;
    SDL_GetRendererOutputSize(renderer, &render_target_w, &render_target_h);

    window_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, render_target_w, render_target_h);

    int pause_text_tex_w, pause_text_tex_h;
    SDL_QueryTexture(pause_tex, NULL, NULL, &pause_text_tex_w, &pause_text_tex_h);

    int pause_tex_w, pause_tex_h;
    SDL_QueryTexture(pause_tex, NULL, NULL, &pause_tex_w, &pause_tex_h);

    if(render_target_w / 2 > pause_tex_w && render_target_h > pause_tex_h){
        pause_render_boundary.w = render_target_w / 2;
        pause_render_boundary.h = (pause_render_boundary.w * pause_tex_h) / pause_tex_w;
        pause_render_boundary.x = (render_target_w / 4);
        pause_render_boundary.y = (render_target_h - pause_render_boundary.h) / 2;
    }
    else{
        pause_render_boundary.w = pause_tex_w;
        pause_render_boundary.h = pause_tex_h;
        pause_render_boundary.x = 0;
        pause_render_boundary.y = 0;
    }
}

chip8_emulator::~chip8_emulator(){
    SDL_DestroyTexture(window_tex);
    SDL_DestroyTexture(frame_tex);
    SDL_DestroyTexture(pause_tex);

    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroyWindow(window);
    window = NULL;
}