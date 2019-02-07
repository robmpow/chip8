#ifndef KEY_HANDLER_IMPL_H
#define KEY_HANDLER_IMPL_H

#include <SDL2/SDL_keycode.h>
#include <array>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>

#include "logger_impl.hpp"
#include "key_handler.h"

enum key_action{KEY_CH8_1 = 0,
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

const std::array<std::string, KEY_EMU_RESET + 1> key_action_string({"KEY_CH8_1",
                                                                    "KEY_CH8_2",
                                                                    "KEY_CH8_3",
                                                                    "KEY_CH8_C",
                                                                    "KEY_CH8_4",
                                                                    "KEY_CH8_5",
                                                                    "KEY_CH8_6",
                                                                    "KEY_CH8_D",
                                                                    "KEY_CH8_7",
                                                                    "KEY_CH8_8",
                                                                    "KEY_CH8_9",
                                                                    "KEY_CH8_E",
                                                                    "KEY_CH8_A",
                                                                    "KEY_CH8_0",
                                                                    "KEY_CH8_B",
                                                                    "KEY_CH8_F",
                                                                    "KEY_EMU_PAUSE",
                                                                    "KEY_EMU_RESET",});

template<class T>
class key_handler_impl : public key_handler<T, key_action, KEY_EMU_RESET + 1>{
private:

public:

    key_handler_impl(std::unordered_map<key_enum, key_action> bind_map) : key_handler<T, key_action, KEY_EMU_RESET + 1>(bind_map){
        for(auto bind_it = this->bind_map.begin(); bind_it!= this->bind_map.end(); bind_it++){
            LOG_TRACE("key_handler: key bound; Bind: '", bind_it->first, "->", key_action_string[bind_it->second], logger::endl);
        }
    };

    void bind_key(key_enum key, key_action action){
        auto bind_it = this->bind_map.find(key);
        if(bind_it != this->bind_map.end()){
            LOG_TRACE("key_handler: key bind replaced; Old Bind: '", key, "->", key_action_string[bind_it->second], "'; New Bind: '", key, "->", key_action_string[action]);
            bind_it->second = action;
        }
        else{
            LOG_TRACE("key_handler: key bound; Bind: '", key, "->", key_action_string[action]);
            this->bind_map.emplace(key, action);
        }
    }

    void bind_keys(std::unordered_map<key_enum, key_action> bind_map){
        bind_map.insert(this->bind_map.begin(), this->bind_map.end());
        this->bind_map = bind_map;
    }

    void bind_action(key_action action, std::function<T> context){
        this->handler_context[action] = context;
    }

    void bind_actions(std::map<key_action, std::function<T>> context_map){
        for(auto it = context_map.begin(); it != context_map.end(); it++){
            this->handler_context[it->first] = it->second;
        }
    }

    bool dispatch(SDL_Keycode key, bool state, bool repeat){
        key_enum pressed_state;
        pressed_state.key = key;
        pressed_state.modifier = SDL_GetModState() & (KMOD_ALT | KMOD_CTRL | KMOD_SHIFT);

        auto bind = this->bind_map.find(pressed_state);
        if(bind != this->bind_map.end()){
            if(this->handler_context[bind->second]){
                this->handler_context[bind->second](state, repeat);
                return true;
            }
        }

        return false;
    }
};

#endif