#ifndef EMU_SDL_H
#define EMU_SDL_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "key_handler_impl.h"
#include "chip8.h"

#define PAUSE_BLINK_INTERVAL 375

enum chip8_run_state{
    RUNNING = 0,
    NOT_RUNNING,
    PAUSED
};

class chip8_emulator{

private:

    SDL_Texture* window_tex;

    SDL_Texture* frame_tex = 0;
    uint32_t frame_tex_fg_color, frame_tex_bg_color;

    SDL_Texture* pause_tex = 0;
    SDL_Rect pause_render_boundary;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Palette* emulator_palette;

    bool running;
    bool chip8_running;
    bool chip8_paused;

    long ticks;

    std::string& rom_path;
    chip8 chip8_instance;
    key_handler_impl<void(bool, bool)> input_handler;

    void render_frame();
    void render_pause(bool force_update);
    void update_sound_state(bool state); 
    void update_target_size();
    void handle_pause(bool press_state, bool repeat);
    void handle_reset(bool press_state, bool repeat);

public:
    chip8_emulator(std::pair<int, int>& resolution, std::pair<SDL_Color, SDL_Color>& palette, std::string& rom_path, std::unordered_map<key_enum, key_action> binds);
    void run();
    ~chip8_emulator();
};

#endif