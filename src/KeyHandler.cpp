#include <SDL2/SDL_keyboard.h>

#include "KeyHandler.hpp"
#include "LoggerImpl.hpp"

namespace Chip8{
    namespace KeyHandler{

        std::string getNameFromKmod(uint16_t t_modifiers){
            const std::array<std::string, 0xF> modifierMap({"LSHIFT",
                                                            "RSHIFT",
                                                            "UNKNOWN",
                                                            "UNKNOWN",
                                                            "UNKNOWN",
                                                            "UNKNOWN",
                                                            "LCTRL",
                                                            "RCTRL",
                                                            "LALT",
                                                            "RALT",
                                                            "LGUI",
                                                            "RGUI",
                                                            "NUM",
                                                            "CAPS",
                                                            "MODE"}); 
            if(!t_modifiers){
                return "Kmod_None";
            }
            std::stringstream ss;
            for(int i = 0; 1 << i <= KMOD_MODE; ++i){
                if(t_modifiers & (1 << i)){
                    if(ss.rdbuf()->in_avail() != 0){
                        ss << "|";
                    }
                    ss << modifierMap[i];
                }
            }
            return ss.str();
        }

        uint16_t getKmodFromName(const std::string& t_modName){
            const std::unordered_map<std::string, uint16_t> kmodMap({{"LSHIFT",   KMOD_LSHIFT},
                                                                     {"RSHIFT",   KMOD_RSHIFT},
                                                                     {"LCTRL",    KMOD_LCTRL},
                                                                     {"RCTRL",    KMOD_RCTRL},
                                                                     {"LALT",     KMOD_LALT},
                                                                     {"RALT",     KMOD_RALT},
                                                                     {"CTRL",     KMOD_CTRL},
                                                                     {"ALT",      KMOD_ALT},
                                                                     {"SHIFT",    KMOD_SHIFT},});

            auto res = kmodMap.find(t_modName);
            if(res != kmodMap.end()){
                return res->second;
            }
            else{
                throw std::string("'" + t_modName + "' does not name an SDL_Keymod");
            }
        }

        std::string getNameFromAction(KeyAction t_keyAction){
            const std::array<std::string, KEY_EMU_RESET + 1> chip8Actions({"key_ch8_1",
                                                                        "key_ch8_2",
                                                                        "key_ch8_3",
                                                                        "key_ch8_c",
                                                                        "key_ch8_4",
                                                                        "key_ch8_5",
                                                                        "key_ch8_6",
                                                                        "key_ch8_d",
                                                                        "key_ch8_7",
                                                                        "key_ch8_8",
                                                                        "key_ch8_9",
                                                                        "key_ch8_e",
                                                                        "key_ch8_a",
                                                                        "key_ch8_0",
                                                                        "key_ch8_b",
                                                                        "key_ch8_f",
                                                                        "key_emu_pause",
                                                                        "key_emu_reset",});
            return chip8Actions[t_keyAction];
        }

        KeyAction getActionFromName(const std::string& t_actionName){
            const std::unordered_map<std::string, KeyAction> chip8Actions({ {"key_ch8_0",       KEY_CH8_0},
                                                                            {"key_ch8_1",       KEY_CH8_1},
                                                                            {"key_ch8_2",       KEY_CH8_2},
                                                                            {"key_ch8_3",       KEY_CH8_3},
                                                                            {"key_ch8_4",       KEY_CH8_4},
                                                                            {"key_ch8_5",       KEY_CH8_5},
                                                                            {"key_ch8_6",       KEY_CH8_6},
                                                                            {"key_ch8_7",       KEY_CH8_7},
                                                                            {"key_ch8_8",       KEY_CH8_8},
                                                                            {"key_ch8_9",       KEY_CH8_9},
                                                                            {"key_ch8_a",       KEY_CH8_A},
                                                                            {"key_ch8_b",       KEY_CH8_B},
                                                                            {"key_ch8_c",       KEY_CH8_C},
                                                                            {"key_ch8_d",       KEY_CH8_D},
                                                                            {"key_ch8_e",       KEY_CH8_E},
                                                                            {"key_ch8_f",       KEY_CH8_F},
                                                                            {"key_emu_pause",   KEY_EMU_PAUSE},
                                                                            {"key_emu_reset",   KEY_EMU_RESET},});

            std::string t_actionNameLower(t_actionName);
            std::transform(t_actionNameLower.begin(), t_actionNameLower.end(), t_actionNameLower.begin(), ::tolower);
            auto res = chip8Actions.find(t_actionNameLower);
            if(res != chip8Actions.end()){
                return res->second;
            }
            else{
                throw std::string("'" + t_actionName + "' does not name a Chip8 emulator action"); 
            }
        }

        std::ostream& operator << (std::ostream& t_os, const KeyPair &t_keyPair){
            t_os << SDL_GetKeyName(t_keyPair.key) << ":" << getNameFromKmod(t_keyPair.modifier);
            return t_os;
        }

        void KeyHandler::bindKey(KeyPair t_keyPair, KeyAction t_keyAction){
            auto res = m_bindMap.find(t_keyPair);
            if(res != m_bindMap.end()){
                chip8Logger.log<Logger::LogTrace>("Chip8::KeyHandler::bindKey, Replaced key binding, Old Binding: '", res->first, "'->'", getNameFromAction(res->second),"'; New Binding: '", res->first, "'->'", getNameFromAction(t_keyAction), "'", Logger::endl);
                res->second = t_keyAction;
            }
            else{
                chip8Logger.log<Logger::LogTrace>("Chip8::KeyHandler::bindKey, New key binding: '", t_keyPair, "'->'", getNameFromAction(t_keyAction), "'", Logger::endl);
                m_bindMap.emplace(t_keyPair, t_keyAction);
            }
        }

        void KeyHandler::bindKeys(std::unordered_map<KeyPair, KeyAction> t_bindMap){
            for(auto bindIt = t_bindMap.begin(); bindIt != t_bindMap.end(); ++bindIt){
                bindKey(bindIt->first, bindIt->second);
            }
        }

        void KeyHandler::bindAction(KeyAction t_keyAction, std::function<void(bool, bool)> t_context){
            if(m_handlerContext[t_keyAction]){
                chip8Logger.log<Logger::LogTrace>("Chip8::KeyHandler::bindAction, Replaced context bind for '", getNameFromAction(t_keyAction), "'", Logger::endl);
            }
            chip8Logger.log<Logger::LogTrace>("Chip8::KeyHandler::bindAction, New context bind for '", getNameFromAction(t_keyAction), "'", Logger::endl);
            m_handlerContext[t_keyAction] = t_context;
        }

        void KeyHandler::bindActions(std::map<KeyAction, std::function<void(bool, bool)>> t_contextMap){
            for(auto bindIt = t_contextMap.begin(); bindIt != t_contextMap.end(); ++bindIt){
                bindAction(bindIt->first, bindIt->second);
            }
        }

        bool KeyHandler::dispatch(SDL_Keycode t_keyCode, bool t_keyState, bool t_keyRepeat){
            KeyPair keyPair{t_keyCode, SDL_GetModState() & ~KMOD_NUM & ~KMOD_CAPS & ~KMOD_GUI};
            auto res = m_bindMap.find(keyPair);
            if(res != m_bindMap.end()){
                if(m_handlerContext[res->second]){
                    m_handlerContext[res->second](t_keyState, t_keyRepeat);
                    return true;
                }
                chip8Logger.log<Logger::LogWarning>("Chip8::KeyHandler::dispatch, Action '", getNameFromAction(res->second), "' is not bound to a context", Logger::endl);
            }
            return false;
        }

    } // namespace KeyHandler
} // namespace Chip8