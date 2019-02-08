#include "key_handler.h"

std::string get_kmod_name(uint16_t mods){
    const std::array<std::string, 0xF> mod_name({"LSHIFT",
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
    if(!mods){
        return "KMOD_NONE";
    }
    std::stringstream ss;
    for(int i = 0; 1 << i <= KMOD_MODE; i++){
        if(mods & (1 << i)){
            if(ss.rdbuf()->in_avail() != 0){
                ss << "|";
            }
            ss << mod_name[i];
        }
    }
    return ss.str();
}

uint16_t lookUpSDLKeymod(std::string mod_name){
    static std::unordered_map<std::string, uint16_t> keymod_map({{"LSHIFT",   KMOD_LSHIFT},
                                                                 {"RSHIFT",   KMOD_RSHIFT},
                                                                 {"LCTRL",    KMOD_LCTRL},
                                                                 {"RCTRL",    KMOD_RCTRL},
                                                                 {"LALT",     KMOD_LALT},
                                                                 {"RALT",     KMOD_RALT},
                                                                 {"CTRL",     KMOD_CTRL},
                                                                 {"ALT",      KMOD_ALT},
                                                                 {"SHIFT",    KMOD_SHIFT},});

    auto res = keymod_map.find(mod_name);
    return (res != keymod_map.end())? res->second :  KMOD_NONE;
}

SDL_Keycode lookUpSDLKeycode(std::string key_name){
    static std::unordered_map<std::string, SDL_Keycode> keycode_map({{"0", SDLK_0},
                                                                     {"1", SDLK_1},
                                                                     {"2", SDLK_2},
                                                                     {"3", SDLK_3},
                                                                     {"4", SDLK_4},
                                                                     {"5", SDLK_5},
                                                                     {"6", SDLK_6},
                                                                     {"7", SDLK_7},
                                                                     {"8", SDLK_8},
                                                                     {"9", SDLK_9},
                                                                     {"A", SDLK_a},
                                                                     {"'", SDLK_QUOTE},
                                                                     {"Application", SDLK_APPLICATION},
                                                                     {"AudioMute", SDLK_AUDIOMUTE},
                                                                     {"AudioNext", SDLK_AUDIONEXT},
                                                                     {"AudioPlay", SDLK_AUDIOPLAY},
                                                                     {"AudioPrev", SDLK_AUDIOPREV},
                                                                     {"AudioStop", SDLK_AUDIOSTOP},
                                                                     {"B", SDLK_b},
                                                                     {"\\", SDLK_BACKSLASH},
                                                                     {"Backspace", SDLK_BACKSPACE},
                                                                     {"BrightnessDown", SDLK_BRIGHTNESSDOWN},
                                                                     {"BrightnessUp", SDLK_BRIGHTNESSUP},
                                                                     {"C", SDLK_c},
                                                                     {"Calculator", SDLK_CALCULATOR},
                                                                     {"Cancel", SDLK_CANCEL},
                                                                     {"CapsLock", SDLK_CAPSLOCK},
                                                                     {"Clear", SDLK_CLEAR},
                                                                     {"Clear/Again", SDLK_CLEARAGAIN},
                                                                     {",", SDLK_COMMA},
                                                                     {"Computer", SDLK_COMPUTER},
                                                                     {"Copy", SDLK_COPY},
                                                                     {"CrSel", SDLK_CRSEL},
                                                                     {"CurrencySubUnit", SDLK_CURRENCYSUBUNIT},
                                                                     {"CurrencyUnit", SDLK_CURRENCYUNIT},
                                                                     {"Cut", SDLK_CUT},
                                                                     {"D", SDLK_d},
                                                                     {"DecimalSeparator", SDLK_DECIMALSEPARATOR},
                                                                     {"Delete", SDLK_DELETE},
                                                                     {"DisplaySwitch", SDLK_DISPLAYSWITCH},
                                                                     {"Down", SDLK_DOWN},
                                                                     {"E", SDLK_e},
                                                                     {"Eject", SDLK_EJECT},
                                                                     {"End", SDLK_END},
                                                                     {"=", SDLK_EQUALS},
                                                                     {"Escape", SDLK_ESCAPE},
                                                                     {"Execute", SDLK_EXECUTE},
                                                                     {"ExSel", SDLK_EXSEL},
                                                                     {"F", SDLK_f},
                                                                     {"F1", SDLK_F1},
                                                                     {"F10", SDLK_F10},
                                                                     {"F11", SDLK_F11},
                                                                     {"F12", SDLK_F12},
                                                                     {"F13", SDLK_F13},
                                                                     {"F14", SDLK_F14},
                                                                     {"F15", SDLK_F15},
                                                                     {"F16", SDLK_F16},
                                                                     {"F17", SDLK_F17},
                                                                     {"F18", SDLK_F18},
                                                                     {"F19", SDLK_F19},
                                                                     {"F2", SDLK_F2},
                                                                     {"F20", SDLK_F20},
                                                                     {"F21", SDLK_F21},
                                                                     {"F22", SDLK_F22},
                                                                     {"F23", SDLK_F23},
                                                                     {"F24", SDLK_F24},
                                                                     {"F3", SDLK_F3},
                                                                     {"F4", SDLK_F4},
                                                                     {"F5", SDLK_F5},
                                                                     {"F6", SDLK_F6},
                                                                     {"F7", SDLK_F7},
                                                                     {"F8", SDLK_F8},
                                                                     {"F9", SDLK_F9},
                                                                     {"Find", SDLK_FIND},
                                                                     {"G", SDLK_g},
                                                                     {"`", SDLK_BACKQUOTE},
                                                                     {"H", SDLK_h},
                                                                     {"Help", SDLK_HELP},
                                                                     {"Home", SDLK_HOME},
                                                                     {"I", SDLK_i},
                                                                     {"Insert", SDLK_INSERT},
                                                                     {"J", SDLK_j},
                                                                     {"K", SDLK_k},
                                                                     {"KBDIllumDown", SDLK_KBDILLUMDOWN},
                                                                     {"KBDIllumToggle", SDLK_KBDILLUMTOGGLE},
                                                                     {"KBDIllumUp", SDLK_KBDILLUMUP},
                                                                     {"Keypad_0", SDLK_KP_0},
                                                                     {"Keypad_00", SDLK_KP_00},
                                                                     {"Keypad_000", SDLK_KP_000},
                                                                     {"Keypad_1", SDLK_KP_1},
                                                                     {"Keypad_2", SDLK_KP_2},
                                                                     {"Keypad_3", SDLK_KP_3},
                                                                     {"Keypad_4", SDLK_KP_4},
                                                                     {"Keypad_5", SDLK_KP_5},
                                                                     {"Keypad_6", SDLK_KP_6},
                                                                     {"Keypad_7", SDLK_KP_7},
                                                                     {"Keypad_8", SDLK_KP_8},
                                                                     {"Keypad_9", SDLK_KP_9},
                                                                     {"Keypad_A", SDLK_KP_A},
                                                                     {"Keypad_&", SDLK_KP_AMPERSAND},
                                                                     {"Keypad_@", SDLK_KP_AT},
                                                                     {"Keypad_B", SDLK_KP_B},
                                                                     {"Keypad_Backspace", SDLK_KP_BACKSPACE},
                                                                     {"Keypad_Binary", SDLK_KP_BINARY},
                                                                     {"Keypad_C", SDLK_KP_C},
                                                                     {"Keypad_Clear", SDLK_KP_CLEAR},
                                                                     {"Keypad_ClearEntry", SDLK_KP_CLEARENTRY},
                                                                     {"Keypad_:", SDLK_KP_COLON},
                                                                     {"Keypad_,", SDLK_KP_COMMA},
                                                                     {"Keypad_D", SDLK_KP_D},
                                                                     {"Keypad_&&", SDLK_KP_DBLAMPERSAND},
                                                                     {"Keypad_||", SDLK_KP_DBLVERTICALBAR},
                                                                     {"Keypad_Decimal", SDLK_KP_DECIMAL},
                                                                     {"Keypad/", SDLK_KP_DIVIDE},
                                                                     {"Keypad_E", SDLK_KP_E},
                                                                     {"Keypad_Enter", SDLK_KP_ENTER},
                                                                     {"Keypad=", SDLK_KP_EQUALS},
                                                                     {"Keypad=(AS400)", SDLK_KP_EQUALSAS400},
                                                                     {"Keypad_!", SDLK_KP_EXCLAM},
                                                                     {"Keypad_F", SDLK_KP_F},
                                                                     {"Keypad_>", SDLK_KP_GREATER},
                                                                     {"Keypad_#", SDLK_KP_HASH},
                                                                     {"Keypad_Hexadecimal", SDLK_KP_HEXADECIMAL},
                                                                     {"Keypad_{", SDLK_KP_LEFTBRACE},
                                                                     {"Keypad_(", SDLK_KP_LEFTPAREN},
                                                                     {"Keypad_<", SDLK_KP_LESS},
                                                                     {"Keypad_MemAdd", SDLK_KP_MEMADD},
                                                                     {"Keypad_MemClear", SDLK_KP_MEMCLEAR},
                                                                     {"Keypad_MemDivide", SDLK_KP_MEMDIVIDE},
                                                                     {"Keypad_MemMultiply", SDLK_KP_MEMMULTIPLY},
                                                                     {"Keypad_MemRecall", SDLK_KP_MEMRECALL},
                                                                     {"Keypad_MemStore", SDLK_KP_MEMSTORE},
                                                                     {"Keypad_MemSubtract", SDLK_KP_MEMSUBTRACT},
                                                                     {"Keypad_-", SDLK_KP_MINUS},
                                                                     {"Keypad_*", SDLK_KP_MULTIPLY},
                                                                     {"Keypad_Octal", SDLK_KP_OCTAL},
                                                                     {"Keypad_%", SDLK_KP_PERCENT},
                                                                     {"Keypad_.", SDLK_KP_PERIOD},
                                                                     {"Keypad_+", SDLK_KP_PLUS},
                                                                     {"Keypad_+/-", SDLK_KP_PLUSMINUS},
                                                                     {"Keypad_^", SDLK_KP_POWER},
                                                                     {"Keypad_}", SDLK_KP_RIGHTBRACE},
                                                                     {"Keypad_)", SDLK_KP_RIGHTPAREN},
                                                                     {"Keypad_Space", SDLK_KP_SPACE},
                                                                     {"Keypad_Tab", SDLK_KP_TAB},
                                                                     {"Keypad_|", SDLK_KP_VERTICALBAR},
                                                                     {"Keypad_XOR", SDLK_KP_XOR},
                                                                     {"L", SDLK_l},
                                                                     {"Left_Alt", SDLK_LALT},
                                                                     {"Left_Ctrl", SDLK_LCTRL},
                                                                     {"Left", SDLK_LEFT},
                                                                     {"[", SDLK_LEFTBRACKET},
                                                                     {"Left_GUI", SDLK_LGUI},
                                                                     {"Left_Shift", SDLK_LSHIFT},
                                                                     {"M", SDLK_m},
                                                                     {"Mail", SDLK_MAIL},
                                                                     {"MediaSelect", SDLK_MEDIASELECT},
                                                                     {"Menu", SDLK_MENU},
                                                                     {"-", SDLK_MINUS},
                                                                     {"ModeSwitch", SDLK_MODE},
                                                                     {"Mute", SDLK_MUTE},
                                                                     {"N", SDLK_n},
                                                                     {"Numlock", SDLK_NUMLOCKCLEAR},
                                                                     {"O", SDLK_o},
                                                                     {"Oper", SDLK_OPER},
                                                                     {"Out", SDLK_OUT},
                                                                     {"P", SDLK_p},
                                                                     {"PageDown", SDLK_PAGEDOWN},
                                                                     {"PageUp", SDLK_PAGEUP},
                                                                     {"Paste", SDLK_PASTE},
                                                                     {"Pause", SDLK_PAUSE},
                                                                     {".", SDLK_PERIOD},
                                                                     {"Power", SDLK_POWER},
                                                                     {"PrintScreen", SDLK_PRINTSCREEN},
                                                                     {"Prior", SDLK_PRIOR},
                                                                     {"Q", SDLK_q},
                                                                     {"R", SDLK_r},
                                                                     {"Right_Alt", SDLK_RALT},
                                                                     {"Right_Ctrl", SDLK_RCTRL},
                                                                     {"Return", SDLK_RETURN},
                                                                     {"Return", SDLK_RETURN2},
                                                                     {"Right_GUI", SDLK_RGUI},
                                                                     {"Right", SDLK_RIGHT},
                                                                     {"]", SDLK_RIGHTBRACKET},
                                                                     {"Right_Shift", SDLK_RSHIFT},
                                                                     {"S", SDLK_s},
                                                                     {"ScrollLock", SDLK_SCROLLLOCK},
                                                                     {"Select", SDLK_SELECT},
                                                                     {";", SDLK_SEMICOLON},
                                                                     {"Separator", SDLK_SEPARATOR},
                                                                     {"/", SDLK_SLASH},
                                                                     {"Sleep", SDLK_SLEEP},
                                                                     {"Space", SDLK_SPACE},
                                                                     {"Stop", SDLK_STOP},
                                                                     {"SysReq", SDLK_SYSREQ},
                                                                     {"T", SDLK_t},
                                                                     {"Tab", SDLK_TAB},
                                                                     {"ThousandsSeparator", SDLK_THOUSANDSSEPARATOR},
                                                                     {"U", SDLK_u},
                                                                     {"Undo", SDLK_UNDO},
                                                                     {"Up", SDLK_UP},
                                                                     {"V", SDLK_v},
                                                                     {"VolumeDown", SDLK_VOLUMEDOWN},
                                                                     {"VolumeUp", SDLK_VOLUMEUP},
                                                                     {"W", SDLK_w},
                                                                     {"WWW", SDLK_WWW},
                                                                     {"X", SDLK_x},
                                                                     {"Y", SDLK_y},
                                                                     {"Z", SDLK_z},
                                                                     {"&", SDLK_AMPERSAND},
                                                                     {"*", SDLK_ASTERISK},
                                                                     {"@", SDLK_AT},
                                                                     {"^", SDLK_CARET},
                                                                     {":", SDLK_COLON},
                                                                     {"$", SDLK_DOLLAR},
                                                                     {"!", SDLK_EXCLAIM},
                                                                     {">", SDLK_GREATER},
                                                                     {"#", SDLK_HASH},
                                                                     {"(", SDLK_LEFTPAREN},
                                                                     {"<", SDLK_LESS},
                                                                     {"%", SDLK_PERCENT},
                                                                     {"+", SDLK_PLUS},
                                                                     {"?", SDLK_QUESTION},
                                                                     {"\"", SDLK_QUOTEDBL},
                                                                     {")", SDLK_RIGHTPAREN},
                                                                     {"_", SDLK_UNDERSCORE},});

    auto res = keycode_map.find(key_name);
    return (res != keycode_map.end())? res->second : SDLK_UNKNOWN;
}