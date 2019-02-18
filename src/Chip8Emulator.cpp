#include <iostream>
#include <iomanip>
#include <SDL2/SDL_endian.h>
#include <chrono>
#include <unistd.h>
#include <functional>
#include <cstdlib>

#include "LoggerImpl.hpp"
#include "Chip8Emulator.hpp"
#include "Chip8Util.hpp"

namespace Chip8{

        template<uint32_t>
        uint32_t mapColorFormat(const SDL_Color& color);

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
        uint32_t mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(const SDL_Color& color){
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

        Emulator::Emulator(const std::pair<int, int>& t_resolution, const std::pair<SDL_Color, SDL_Color>& t_palette, std::string& t_romPath, std::unordered_map<KeyHandler::KeyPair, KeyHandler::KeyAction> t_keyBinds, bool t_chip8Seed) : m_run(true), 
                                                                                                                                                                                                                m_chip8Run(true), 
                                                                                                                                                                                                                m_chip8Paused(false), 
                                                                                                                                                                                                                m_romPath(t_romPath), 
                                                                                                                                                                                                                m_chip8Instance((t_chip8Seed)? std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) : 0, t_romPath), 
                                                                                                                                                                                                                m_inputHandler(t_keyBinds){

            this->m_window = SDL_CreateWindow("Chip8 Emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, t_resolution.first, t_resolution.second, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
            chip8Logger.log<Logger::LogTrace>("Emulator: m_window created", Logger::endl);

            this->m_renderer = SDL_CreateRenderer( m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
            chip8Logger.log<Logger::LogTrace>("Emulator: m_renderer created", Logger::endl);

            int render_target_w, render_target_h;
            SDL_GetRendererOutputSize(m_renderer, &render_target_w, &render_target_h);

            m_windowTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, render_target_w, render_target_h);

            m_frameFgColor =    mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(t_palette.first);

            m_frameBgColor =    mapColorFormat<SDL_PIXELFORMAT_RGBA8888>(t_palette.second);

            m_frameTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

            int bpp;
            uint32_t r_mask, g_mask, b_mask, a_mask;
            SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &r_mask, &g_mask, &b_mask, &a_mask);

            SDL_Surface* pauseSurfaceBmp = SDL_LoadBMP("./res/ch8_paused.bmp");
            SDL_Surface* pauseSurface = SDL_ConvertSurfaceFormat(pauseSurfaceBmp, SDL_PIXELFORMAT_RGBA8888, 0);
            SDL_FreeSurface(pauseSurfaceBmp);

            uint32_t pauseBmpFgColor = SDL_MapRGBA(pauseSurface->format, 0xFF, 0xFF, 0xFF, 0xFF);
            uint32_t pauseBmpBgColor = SDL_MapRGBA(pauseSurface->format, 0x00, 0x00, 0x00, 0xFF);

            if(m_frameFgColor != pauseBmpFgColor || m_frameBgColor != pauseBmpBgColor){
                if(!SDL_LockSurface(pauseSurface)){
                    uint32_t* pixels = static_cast<uint32_t*>(pauseSurface->pixels);
                    for(int j = 0; j < pauseSurface->h; j++){
                        for(int i = 0; i < pauseSurface->w; i++){
                            if(pixels[i + j * (pauseSurface->pitch / sizeof(uint32_t))] == pauseBmpFgColor){
                                pixels[i + j * (pauseSurface->pitch / sizeof(uint32_t))] = m_frameFgColor;
                            }
                            if(pixels[i + j * (pauseSurface->pitch / sizeof(uint32_t))] == pauseBmpBgColor){
                                pixels[i + j * (pauseSurface->pitch / sizeof(uint32_t))] = m_frameBgColor;
                            }
                        }
                    }
                    SDL_UnlockSurface(pauseSurface);
                }
            }

            m_pauseTexture = SDL_CreateTextureFromSurface(m_renderer, pauseSurface);

            SDL_FreeSurface(pauseSurface);

            int pauseTextureW, pauseTextureH;
            SDL_QueryTexture(m_pauseTexture, NULL, NULL, &pauseTextureW, &pauseTextureH);

            if(render_target_w / 2 > pauseTextureW && render_target_h > pauseTextureH){
                m_pauseRenderBoundary.w = render_target_w / 2;
                m_pauseRenderBoundary.h = (m_pauseRenderBoundary.w * pauseTextureH) / pauseTextureW;
                m_pauseRenderBoundary.x = (render_target_w / 4);
                m_pauseRenderBoundary.y = (render_target_h - m_pauseRenderBoundary.h) / 2;
            }
            else{
                m_pauseRenderBoundary.w = pauseTextureW;
                m_pauseRenderBoundary.h = pauseTextureH;
                m_pauseRenderBoundary.x = 0;
                m_pauseRenderBoundary.y = 0;
            }

            m_inputHandler.bindAction(KeyHandler::KEY_CH8_0, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_0));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_1, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_1));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_2, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_2));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_3, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_3));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_4, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_4));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_5, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_5));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_6, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_6));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_7, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_7));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_8, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_8));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_9, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_9));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_A, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_A));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_B, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_B));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_C, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_C));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_D, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_D));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_E, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_E));
            m_inputHandler.bindAction(KeyHandler::KEY_CH8_F, std::bind(&Chip8::updateKeystate, &m_chip8Instance, arg::_1, arg::_2, KEY_F));
            m_inputHandler.bindAction(KeyHandler::KEY_EMU_RESET, std::bind(&Emulator::handleResetInput, this, arg::_1, arg::_2));
            m_inputHandler.bindAction(KeyHandler::KEY_EMU_PAUSE, std::bind(&Emulator::handlePauseInput, this, arg::_1, arg::_2));
        }

        void Emulator::handlePauseInput(bool t_pressState, bool t_repeat){

            if(t_pressState && !t_repeat){
                m_chip8Paused = !m_chip8Paused;
            }

            if(m_chip8Paused){
                renderPause(true);
            }
            else{
                renderFrame();
            }
        }

        void Emulator::handleResetInput(bool t_pressState, bool t_repeat){
            if(t_pressState && !t_repeat){
                m_chip8Instance.reset();
                m_chip8Instance.load(m_romPath);
                m_chip8Run = true;
            }
        }

        void Emulator::run(){

            SDL_Event e;

            std::chrono::high_resolution_clock::time_point beg, end;
            while(m_run){
                beg = std::chrono::high_resolution_clock::now();
                if(m_run && m_chip8Run && !m_chip8Paused){
                    try{
                        TickResult res = m_chip8Instance.run_tick();
                        if(res.displayUpdate){
                            renderFrame();
                        }
                        updateSoundState(res.soundState);
                    }
                    catch(std::string error_msg){
                        chip8Logger.log<Logger::LogError>(error_msg, Logger::endl);
                        m_chip8Run = false;
                    }
                }
                else if(m_chip8Paused){
                    renderPause(false);
                }
                while(m_run && SDL_PollEvent(&e)){
                    switch(e.type){
                        case SDL_QUIT:
                            m_run = false;
                            break;
                        case SDL_KEYDOWN:
                        case SDL_KEYUP:
                            m_inputHandler.dispatch(e.key.keysym.sym, e.type == SDL_KEYDOWN, e.key.repeat);
                            break;
                        case SDL_WINDOWEVENT:
                            switch(e.window.event){
                                case SDL_WINDOWEVENT_SIZE_CHANGED:
                                case SDL_WINDOWEVENT_RESIZED:
                                    updateTargetSize();
                                    break;
                                case SDL_WINDOWEVENT_EXPOSED:
                                    if(m_chip8Paused){
                                        renderPause(true);
                                    }
                                    else{
                                        renderFrame();
                                    }
                            }
                            break;
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                m_ticks++;
                auto sleep_duration = CHIP8_TICK_PERIOD_USEC - std::chrono::duration_cast<std::chrono::microseconds>(end - beg).count();
                if(sleep_duration > 0){
                    usleep(sleep_duration);
                }
            }
        }

        void Emulator::renderFrame(){
            uint32_t* frame_tex_pixels = 0;
            int frame_tex_pitch = 0;

            if(!SDL_LockTexture(m_frameTexture, NULL, (void**)&frame_tex_pixels, &frame_tex_pitch)){
                auto ch8_display = m_chip8Instance.getDisplay();
                for(uint ind = 0; ind < CHIP8_DISP_SIZE; ind++){
                    uint8_t disp_byte = ch8_display[ind];
                    for(uint bit_ind = 0; bit_ind < 8; bit_ind++){
                        frame_tex_pixels[ind * 8 + (7 - bit_ind)] = (disp_byte & 0x01)? m_frameFgColor : m_frameBgColor;
                        disp_byte >>= 1;
                    }
                }

            }
            frame_tex_pixels = 0;
            SDL_UnlockTexture(m_frameTexture);

            SDL_BlendMode renderer_blendmode;
            SDL_GetRenderDrawBlendMode(m_renderer, &renderer_blendmode);

            SDL_RenderCopy(m_renderer, m_frameTexture, NULL, NULL);
            SDL_RenderPresent(m_renderer);

        }

        void Emulator::renderPause(bool force_update){

            static long lastBlinkTicks = m_ticks;
            static bool show_message = true;

            if(force_update){
                lastBlinkTicks = m_ticks;
                show_message = true;
            }

            if((m_ticks - lastBlinkTicks > PAUSE_BLINK_INTERVAL) || force_update){
                SDL_SetRenderTarget(m_renderer, m_windowTexture);

                SDL_RenderClear(m_renderer);

                SDL_SetTextureAlphaMod(m_frameTexture, 0x7F);
                SDL_SetTextureBlendMode(m_frameTexture, SDL_BLENDMODE_BLEND);
                SDL_RenderCopy(m_renderer, m_frameTexture, NULL, NULL);
                SDL_SetTextureAlphaMod(m_frameTexture, 0xFF);
                SDL_SetTextureBlendMode(m_frameTexture, SDL_BLENDMODE_BLEND);

                if(show_message)
                    SDL_RenderCopy(m_renderer, m_pauseTexture, NULL, &m_pauseRenderBoundary);

                SDL_SetRenderTarget(m_renderer, NULL);
                SDL_RenderCopy(m_renderer, m_windowTexture, NULL, NULL);
                SDL_RenderPresent(m_renderer);
            }

            if(m_ticks - lastBlinkTicks > PAUSE_BLINK_INTERVAL){
                show_message = !show_message;
                lastBlinkTicks = m_ticks;
            }
        }

        void Emulator::updateSoundState(bool state){
            return;
        }

        void Emulator::updateTargetSize(){
            SDL_DestroyTexture(m_windowTexture);

            int render_target_w, render_target_h;
            SDL_GetRendererOutputSize(m_renderer, &render_target_w, &render_target_h);

            m_windowTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, render_target_w, render_target_h);

            int pause_text_tex_w, pause_text_tex_h;
            SDL_QueryTexture(m_pauseTexture, NULL, NULL, &pause_text_tex_w, &pause_text_tex_h);

            int pauseTextureW, pauseTextureH;
            SDL_QueryTexture(m_pauseTexture, NULL, NULL, &pauseTextureW, &pauseTextureH);

            if(render_target_w / 2 > pauseTextureW && render_target_h > pauseTextureH){
                m_pauseRenderBoundary.w = render_target_w / 2;
                m_pauseRenderBoundary.h = (m_pauseRenderBoundary.w * pauseTextureH) / pauseTextureW;
                m_pauseRenderBoundary.x = (render_target_w / 4);
                m_pauseRenderBoundary.y = (render_target_h - m_pauseRenderBoundary.h) / 2;
            }
            else{
                m_pauseRenderBoundary.w = pauseTextureW;
                m_pauseRenderBoundary.h = pauseTextureH;
                m_pauseRenderBoundary.x = 0;
                m_pauseRenderBoundary.y = 0;
            }
        }

        Emulator::~Emulator(){
            SDL_DestroyTexture(m_windowTexture);
            SDL_DestroyTexture(m_frameTexture);
            SDL_DestroyTexture(m_pauseTexture);

            SDL_DestroyRenderer(m_renderer);
            m_renderer = NULL;
            SDL_DestroyWindow(m_window);
            m_window = NULL;
        }
        
} // namespace Chip8