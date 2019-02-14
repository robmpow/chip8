#ifndef EMU_SDL_H
#define EMU_SDL_H

#include <SDL2/SDL.h>
#include <cstdint>
#include <functional>
#include <utility>

#include "KeyHandler.hpp"
#include "Chip8.hpp"

#define PAUSE_BLINK_INTERVAL 375

namespace Chip8{

        enum chip8_run_state{
            RUNNING = 0,
            NOT_RUNNING,
            PAUSED
        };

        class Emulator{

        private:

            SDL_Texture* m_windowTexture;

            SDL_Texture* m_frameTexture = nullptr;
            uint32_t m_frameBgColor, m_frameFgColor;

            SDL_Texture* m_pauseTexture = nullptr;
            SDL_Rect m_pauseRenderBoundary;

            SDL_Window* m_window;
            SDL_Renderer* m_renderer;

            SDL_Palette* m_palette;

            bool m_run;
            bool m_chip8Run;
            bool m_chip8Paused;

            long m_ticks;

            const std::string& m_romPath;
            Chip8 m_chip8Instance;
            KeyHandler::KeyHandler m_inputHandler;

            void renderFrame();
            void renderPause(bool t_forceUpdate);
            void updateSoundState(bool t_state); 
            void updateTargetSize();
            void handlePauseInput(bool t_state, bool t_repeat);
            void handleResetInput(bool t_state, bool t_repeat);

        public:
            Emulator(const std::pair<int, int>& t_resolution, const std::pair<SDL_Color, SDL_Color>& t_palette, std::string& t_romPath, std::unordered_map<KeyHandler::KeyPair, KeyHandler::KeyAction> t_keyBinds, bool t_chip8Seed);
            void run();
            ~Emulator();
        };

} // namespace Chip8

#endif