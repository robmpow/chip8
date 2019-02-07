#ifndef KEY_HANDLER_H
#define KEY_HANDLER_H

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_keyboard.h>
#include <iostream>
#include <sstream>

std::string get_kmod_name(uint16_t mods);

struct key_enum{
    SDL_Keycode key;
    uint16_t modifier;

    bool operator==(const key_enum& other) const{
        return key == other.key && (modifier == other.modifier || (modifier & other.modifier));
    }
    template<typename T>
    friend std::ostream& operator << (std::ostream& os, T &t){
        os << SDL_GetKeyName(t.key) << ":" << get_kmod_name(t.modifier);
        return os;
    }
};

template<>
struct std::hash<key_enum>{
    std::size_t operator()(const key_enum& k) const {
        return hash<SDL_Keycode>()(k.key);
    }
};

SDL_Keycode lookUpSDLKeycode(std::string key_name);
uint16_t lookUpSDLKeymod(std::string mod_name);

template<class T, typename U, size_t MAX_ACTIONS>
class key_handler{

protected:
    std::unordered_map<key_enum, U> bind_map;
    std::array<std::function<T>, MAX_ACTIONS> handler_context;

public:
    key_handler(std::unordered_map<key_enum, U> bind_map) : bind_map(bind_map), handler_context(){};
    virtual void bind_key(key_enum, U) = 0;
    virtual void bind_keys(std::unordered_map<key_enum, U>) = 0;
    virtual void bind_action(U, std::function<T>) = 0;
    virtual void bind_actions(std::map<U, std::function<T>>) = 0;
    virtual bool dispatch(SDL_Keycode, bool, bool) = 0;
};

#endif