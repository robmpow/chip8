#ifndef KEY_HANDLER_KEY_HANDLER_HPP
#define KEY_HANDLER_KEY_HANDLER_HPP

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>

namespace Chip8{

struct KeyPair{
    SDL_Keycode key;
    uint16_t modifier;

    bool operator==(const KeyPair& t_other) const{
        return key == t_other.key && (modifier == t_other.modifier || (modifier & t_other.modifier));
    }

    friend std::ostream& operator << (std::ostream& t_os, const KeyPair &t_keyPair);
};

} // namespace Chip8

template<>
struct std::hash<Chip8::KeyPair>{
    std::size_t operator()(const Chip8::KeyPair& t_KeyPair) const {
        return std::hash<SDL_Keycode>()(t_KeyPair.key) ^ std::hash<uint16_t>()(t_KeyPair.modifier);
    }
};


namespace Chip8{

enum KeyAction{
    KEY_CH8_1 = 0,
    KEY_CH8_2,
    KEY_CH8_3,
    KEY_CH8_C,
    KEY_CH8_4,
    KEY_CH8_5,
    KEY_CH8_6,
    KEY_CH8_D,
    KEY_CH8_7,
    KEY_CH8_8,
    KEY_CH8_9,
    KEY_CH8_E,
    KEY_CH8_A,
    KEY_CH8_0,
    KEY_CH8_B,
    KEY_CH8_F,
    KEY_EMU_PAUSE,
    KEY_EMU_RESET,
};

class KeyHandler{

protected:
    std::unordered_map<KeyPair, KeyAction> m_bindMap;
    std::array<std::function<void(bool ,bool)>, KEY_EMU_RESET + 1> m_handlerContext;

public:
    KeyHandler() : m_bindMap(), m_handlerContext({nullptr}) {};
    KeyHandler(const std::unordered_map<KeyPair, KeyAction>& t_bindMap) : m_bindMap(t_bindMap), m_handlerContext({nullptr}){};

    void bindKey(KeyPair t_keyPair, KeyAction t_keyAction);
    void bindKeys(std::unordered_map<KeyPair, KeyAction> t_bindMap);
    void bindAction(KeyAction t_keyAction, std::function<void(bool, bool)> t_context);
    void bindActions(std::map<KeyAction, std::function<void(bool, bool)>> t_contextMap);
    bool dispatch(SDL_Keycode t_keyCode, bool t_keyState, bool t_keyRepeat);

    static std::string getNameFromKmod(uint16_t t_modifiers);
    static uint16_t getKmodFromName(const std::string& t_keyName);
    static std::string getNameFromAction(KeyAction t_keyAction);
    static KeyAction getActionFromName(const std::string& t_actionName);
};

} // namespace Chip8

#endif // KEY_HANDLER_KEY_HANDLER_HPP