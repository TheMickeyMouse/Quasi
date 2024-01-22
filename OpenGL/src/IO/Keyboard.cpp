#include <algorithm>
// including glew before gl
#include "GraphicsDevice.h"

#include "Keyboard.h"

/*
* reference *
* #define GLFW_KEY_UNKNOWN            -1
* #define GLFW_KEY_SPACE              32
* #define GLFW_KEY_APOSTROPHE         39 * ' *
* #define GLFW_KEY_COMMA              44 * , *
* #define GLFW_KEY_MINUS              45 * - *
* #define GLFW_KEY_PERIOD             46 * . *
* #define GLFW_KEY_SLASH              47 * / *
* #define GLFW_KEY_0                  48
* #define GLFW_KEY_1                  49
* #define GLFW_KEY_2                  50
* #define GLFW_KEY_3                  51
* #define GLFW_KEY_4                  52
* #define GLFW_KEY_5                  53
* #define GLFW_KEY_6                  54
* #define GLFW_KEY_7                  55
* #define GLFW_KEY_8                  56
* #define GLFW_KEY_9                  57
* #define GLFW_KEY_SEMICOLON          59 * ; *
* #define GLFW_KEY_EQUAL              61 * = *
* #define GLFW_KEY_A                  65
* #define GLFW_KEY_B                  66
* #define GLFW_KEY_C                  67
* #define GLFW_KEY_D                  68
* #define GLFW_KEY_E                  69
* #define GLFW_KEY_F                  70
* #define GLFW_KEY_G                  71
* #define GLFW_KEY_H                  72
* #define GLFW_KEY_I                  73
* #define GLFW_KEY_J                  74
* #define GLFW_KEY_K                  75
* #define GLFW_KEY_L                  76
* #define GLFW_KEY_M                  77
* #define GLFW_KEY_N                  78
* #define GLFW_KEY_O                  79
* #define GLFW_KEY_P                  80
* #define GLFW_KEY_Q                  81
* #define GLFW_KEY_R                  82
* #define GLFW_KEY_S                  83
* #define GLFW_KEY_T                  84
* #define GLFW_KEY_U                  85
* #define GLFW_KEY_V                  86
* #define GLFW_KEY_W                  87
* #define GLFW_KEY_X                  88
* #define GLFW_KEY_Y                  89
* #define GLFW_KEY_Z                  90
* #define GLFW_KEY_LEFT_BRACKET       91  * [ *
* #define GLFW_KEY_BACKSLASH          92  * \ *
* #define GLFW_KEY_RIGHT_BRACKET      93  * ] *
* #define GLFW_KEY_GRAVE_ACCENT       96  * ` *
* #define GLFW_KEY_WORLD_1            161 * non-US #1 *
* #define GLFW_KEY_WORLD_2            162 * non-US #2 *
* Function keys *
* #define GLFW_KEY_ESCAPE             256
* #define GLFW_KEY_ENTER              257
* #define GLFW_KEY_TAB                258
* #define GLFW_KEY_BACKSPACE          259
* #define GLFW_KEY_INSERT             260
* #define GLFW_KEY_DELETE             261
* #define GLFW_KEY_RIGHT              262
* #define GLFW_KEY_LEFT               263
* #define GLFW_KEY_DOWN               264
* #define GLFW_KEY_UP                 265
* #define GLFW_KEY_PAGE_UP            266
* #define GLFW_KEY_PAGE_DOWN          267
* #define GLFW_KEY_HOME               268
* #define GLFW_KEY_END                269
* #define GLFW_KEY_CAPS_LOCK          280
* #define GLFW_KEY_SCROLL_LOCK        281
* #define GLFW_KEY_NUM_LOCK           282
* #define GLFW_KEY_PRINT_SCREEN       283
* #define GLFW_KEY_PAUSE              284
* #define GLFW_KEY_F1                 290
* #define GLFW_KEY_F2                 291
* #define GLFW_KEY_F3                 292
* #define GLFW_KEY_F4                 293
* #define GLFW_KEY_F5                 294
* #define GLFW_KEY_F6                 295
* #define GLFW_KEY_F7                 296
* #define GLFW_KEY_F8                 297
* #define GLFW_KEY_F9                 298
* #define GLFW_KEY_F10                299
* #define GLFW_KEY_F11                300
* #define GLFW_KEY_F12                301
* #define GLFW_KEY_F13                302
* #define GLFW_KEY_F14                303
* #define GLFW_KEY_F15                304
* #define GLFW_KEY_F16                305
* #define GLFW_KEY_F17                306
* #define GLFW_KEY_F18                307
* #define GLFW_KEY_F19                308
* #define GLFW_KEY_F20                309
* #define GLFW_KEY_F21                310
* #define GLFW_KEY_F22                311
* #define GLFW_KEY_F23                312
* #define GLFW_KEY_F24                313
* #define GLFW_KEY_F25                314
* #define GLFW_KEY_KP_0               320
* #define GLFW_KEY_KP_1               321
* #define GLFW_KEY_KP_2               322
* #define GLFW_KEY_KP_3               323
* #define GLFW_KEY_KP_4               324
* #define GLFW_KEY_KP_5               325
* #define GLFW_KEY_KP_6               326
* #define GLFW_KEY_KP_7               327
* #define GLFW_KEY_KP_8               328
* #define GLFW_KEY_KP_9               329
* #define GLFW_KEY_KP_DECIMAL         330
* #define GLFW_KEY_KP_DIVIDE          331
* #define GLFW_KEY_KP_MULTIPLY        332
* #define GLFW_KEY_KP_SUBTRACT        333
* #define GLFW_KEY_KP_ADD             334
* #define GLFW_KEY_KP_ENTER           335
* #define GLFW_KEY_KP_EQUAL           336
* #define GLFW_KEY_LEFT_SHIFT         340
* #define GLFW_KEY_LEFT_CONTROL       341
* #define GLFW_KEY_LEFT_ALT           342
* #define GLFW_KEY_LEFT_SUPER         343
* #define GLFW_KEY_RIGHT_SHIFT        344
* #define GLFW_KEY_RIGHT_CONTROL      345
* #define GLFW_KEY_RIGHT_ALT          346
* #define GLFW_KEY_RIGHT_SUPER        347
* #define GLFW_KEY_MENU               348
* #define GLFW_KEY_LAST               GLFW_KEY_MENU
*/

namespace IO {
    OPENGL_API KeyboardT Keyboard { nullptr };
    
    GLFWwindow* KeyboardT::inputWindow() { return graphicsDevice->GetWindow(); }
    const GLFWwindow* KeyboardT::inputWindow() const { return graphicsDevice->GetWindow(); }
    
    KeyboardT::KeyboardT(Graphics::GraphicsDevice& gd) : graphicsDevice(gd) {
        glfwSetKeyCallback(inputWindow(),
            // clever hack >:)
            [](auto... args) { Keyboard.OnGlfwKeyCallback(args...); } );
    }

    void KeyboardT::Update() {
        prevKeySet = currKeySet;
        for (KeyIndex ki : unadded) setKeyStatusOf(currKeySet, ki & 127, ki & 128);
        unadded.clear();
    }

    bool KeyboardT::getKeyStatusOf(const Keyset& ks, KeyIndex ki)     { return ks[ki / 64ULL] & 1ULL << (ki & 63); }
    void KeyboardT::setKeyStatusOf(Keyset& ks, KeyIndex ki, bool val) { ks[ki / 64ULL] = (ks[ki / 64ULL] & ~(1ULL << (ki & 63))) | (uint64)val << (ki & 63); }

    void KeyboardT::OnGlfwKeyCallback(GLFWwindow* window, int key, int positionCode, int action, int modifierBits) {
        const bool isPress = action != GLFW_RELEASE;
        unadded.push_back((uchar)((isPress << 7) | ToKeyIndex((Key)key)));
    }

    bool KeyboardT::KeyPressed(Key key)   const { return getCurrKeyStatus(key); }
    bool KeyboardT::KeyOnPress(Key key)   const { return !getPrevKeyStatus(key) && getCurrKeyStatus(key); }
    bool KeyboardT::KeyOnRelease(Key key) const { return !getCurrKeyStatus(key) && getPrevKeyStatus(key); }
    bool KeyboardT::AnyPressed()  const { return std::ranges::any_of (currKeySet, [](auto x){ return x != 0; }); }
    bool KeyboardT::NonePressed() const { return std::ranges::none_of(currKeySet, [](auto x){ return x != 0; }); }
    
    std::vector<Key> KeyboardT::KeysPressed() const {
        std::vector<Key> keys = {};
        uchar chunki = 0;
        for (const auto chunk64 : currKeySet) {
            for (uchar i = 0; i < 64; ++i) {
                if (chunk64 & 1ULL << i) keys.push_back(FromKeyIndex(i + chunki * 64));
            }
            ++chunki;
        }
        return keys;
    }

    bool KeyboardT::IsValidKey(Key key) {
        return ToKeyIndex(key) != 255;
        // idk man prob not the best but whatev
    }

    // this is the worst code ive ever written
    // but i dont see any other option
    KeyIndex KeyboardT::ToKeyIndex(Key key) {
        using enum Key;
        int offset = 0;
#define RANGE_MATCH(S, E) if ((S) <= key && key <= (E)) return (int)(key - (S)) + offset; offset += (int)(((E) - (S)) + 1)
        
        // chars
        RANGE_MATCH(A, Z);
        // nums
        RANGE_MATCH(NUM_0, NUM_9);
        
        // puncs
        RANGE_MATCH(SPACE, QUOTATIONS);
        RANGE_MATCH(COMMA, SLASH);
        RANGE_MATCH(SEMICOLON, EQUAL);
        RANGE_MATCH(OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET);
        RANGE_MATCH(GRAVE_ACCENT, GRAVE_ACCENT);

        // keypads
        RANGE_MATCH(KEYPAD_0, KEYPAD_EQUAL);

        // funs
        RANGE_MATCH(F1, F25);

        // utils
        RANGE_MATCH(ESCAPE, END);
        RANGE_MATCH(CAPS_LOCK, PAUSE);
        RANGE_MATCH(LSHIFT, MENU);

        // non us
        RANGE_MATCH(NON_US_1, NON_US_2);
        
        return 255;
    }
#undef RANGE_MATCH

    Key KeyboardT::FromKeyIndex(KeyIndex ki) {
        using enum Key;

#define RANGE_MATCH(S, E) if (ToKeyIndex((S)) <= ki && ki <= ToKeyIndex((E))) return ki - ToKeyIndex((S)) + (S)

        // chars
        RANGE_MATCH(A, Z);

        // nums
        RANGE_MATCH(NUM_0, NUM_9);

        // puncs
        RANGE_MATCH(SPACE, QUOTATIONS);
        RANGE_MATCH(COMMA, SLASH);
        RANGE_MATCH(SEMICOLON, EQUAL);
        RANGE_MATCH(OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET);
        RANGE_MATCH(GRAVE_ACCENT, GRAVE_ACCENT);

        // keypads
        RANGE_MATCH(KEYPAD_0, KEYPAD_EQUAL);

        // funs
        RANGE_MATCH(F1, F25);

        // utils
        RANGE_MATCH(ESCAPE, END);
        RANGE_MATCH(CAPS_LOCK, PAUSE);
        RANGE_MATCH(LSHIFT, MENU);

        // non us
        RANGE_MATCH(NON_US_1, NON_US_2);

        return UNKNOWN;
    }
#undef RANGE_MATCH

    Key KeyboardT::FromModBits(ModifierKey mod) {
#define MOD_KEY_CASE(M) case ModifierKey::M: return Key::L##M
        switch (mod) {
            MOD_KEY_CASE(SHIFT);
            MOD_KEY_CASE(CONTROL);
            MOD_KEY_CASE(ALT);
            MOD_KEY_CASE(SUPER);
            case ModifierKey::CAPS_LOCK: return Key::CAPS_LOCK;
            case ModifierKey::NUM_LOCK:  return Key::NUM_LOCK;
        }
        return Key::UNKNOWN;
#undef MOD_KEY_CASE
    }

    KeyIndex KeyboardT::IndexFromModBits(ModifierKey mod) {
        return ToKeyIndex(FromModBits(mod));
    }
    
    const char* KeyboardT::KeyToStr(Key key) {
#define BASIC_KEY_CASE(K) case Key::K: return #K
#define PS_KEY_CASE(PK, PN, K) case Key::PK##K: return #PN#K
#define SPECIAL_KEY_CASE(K, N) case Key::K: return N

        switch (key) {
            BASIC_KEY_CASE(A);
            BASIC_KEY_CASE(B);
            BASIC_KEY_CASE(C);
            BASIC_KEY_CASE(D);
            BASIC_KEY_CASE(E);
            BASIC_KEY_CASE(F);
            BASIC_KEY_CASE(G);
            BASIC_KEY_CASE(H);
            BASIC_KEY_CASE(I);
            BASIC_KEY_CASE(J);
            BASIC_KEY_CASE(K);
            BASIC_KEY_CASE(L);
            BASIC_KEY_CASE(M);
            BASIC_KEY_CASE(N);
            BASIC_KEY_CASE(O);
            BASIC_KEY_CASE(P);
            BASIC_KEY_CASE(Q);
            BASIC_KEY_CASE(R);
            BASIC_KEY_CASE(S);
            BASIC_KEY_CASE(T);
            BASIC_KEY_CASE(U);
            BASIC_KEY_CASE(V);
            BASIC_KEY_CASE(W);
            BASIC_KEY_CASE(X);
            BASIC_KEY_CASE(Y);
            BASIC_KEY_CASE(Z);

            PS_KEY_CASE(NUM_,,0);
            PS_KEY_CASE(NUM_,,1);
            PS_KEY_CASE(NUM_,,2);
            PS_KEY_CASE(NUM_,,3);
            PS_KEY_CASE(NUM_,,4);
            PS_KEY_CASE(NUM_,,5);
            PS_KEY_CASE(NUM_,,6);
            PS_KEY_CASE(NUM_,,7);
            PS_KEY_CASE(NUM_,,8);
            PS_KEY_CASE(NUM_,,9);

            SPECIAL_KEY_CASE(SPACE,                "Space");
            SPECIAL_KEY_CASE(QUOTATIONS,           "'");
            SPECIAL_KEY_CASE(COMMA,                ",");
            SPECIAL_KEY_CASE(MINUS,                "-");
            SPECIAL_KEY_CASE(PERIOD,               ".");
            SPECIAL_KEY_CASE(SLASH,                "/");
            SPECIAL_KEY_CASE(SEMICOLON,            ":");
            SPECIAL_KEY_CASE(EQUAL,                "=");
            SPECIAL_KEY_CASE(OPEN_SQUARE_BRACKET,  "[");
            SPECIAL_KEY_CASE(BACK_SLASH,           "\\");
            SPECIAL_KEY_CASE(CLOSE_SQUARE_BRACKET, "]");
            SPECIAL_KEY_CASE(GRAVE_ACCENT,         "`");

            PS_KEY_CASE(KEYPAD_,Keypad ,0);
            PS_KEY_CASE(KEYPAD_,Keypad ,1);
            PS_KEY_CASE(KEYPAD_,Keypad ,2);
            PS_KEY_CASE(KEYPAD_,Keypad ,3);
            PS_KEY_CASE(KEYPAD_,Keypad ,4);
            PS_KEY_CASE(KEYPAD_,Keypad ,5);
            PS_KEY_CASE(KEYPAD_,Keypad ,6);
            PS_KEY_CASE(KEYPAD_,Keypad ,7);
            PS_KEY_CASE(KEYPAD_,Keypad ,8);
            PS_KEY_CASE(KEYPAD_,Keypad ,9);
            SPECIAL_KEY_CASE(KEYPAD_DECIMAL,  "Keypad .");
            SPECIAL_KEY_CASE(KEYPAD_DIVIDE,   "Keypad /");
            SPECIAL_KEY_CASE(KEYPAD_MULTIPLY, "Keypad *");
            SPECIAL_KEY_CASE(KEYPAD_SUBTRACT, "Keypad -");
            SPECIAL_KEY_CASE(KEYPAD_ADD,      "Keypad +");
            SPECIAL_KEY_CASE(KEYPAD_ENTER,    "Keypad Enter");
            SPECIAL_KEY_CASE(KEYPAD_EQUAL,    "Keypad =");

            BASIC_KEY_CASE(F1);
            BASIC_KEY_CASE(F2);
            BASIC_KEY_CASE(F3);
            BASIC_KEY_CASE(F4);
            BASIC_KEY_CASE(F5);
            BASIC_KEY_CASE(F6);
            BASIC_KEY_CASE(F7);
            BASIC_KEY_CASE(F8);
            BASIC_KEY_CASE(F9);
            BASIC_KEY_CASE(F10);
            BASIC_KEY_CASE(F11);
            BASIC_KEY_CASE(F12);
            BASIC_KEY_CASE(F13);
            BASIC_KEY_CASE(F14);
            BASIC_KEY_CASE(F15);
            BASIC_KEY_CASE(F16);
            BASIC_KEY_CASE(F17);
            BASIC_KEY_CASE(F18);
            BASIC_KEY_CASE(F19);
            BASIC_KEY_CASE(F20);
            BASIC_KEY_CASE(F21);
            BASIC_KEY_CASE(F22);
            BASIC_KEY_CASE(F23);
            BASIC_KEY_CASE(F24);
            BASIC_KEY_CASE(F25);

            SPECIAL_KEY_CASE(ESCAPE,       "Escape");
            SPECIAL_KEY_CASE(ENTER,        "Enter");
            SPECIAL_KEY_CASE(TAB,          "Tab");
            SPECIAL_KEY_CASE(BACKSPACE,    "Backspace");
            SPECIAL_KEY_CASE(INSERT,       "Insert");
            SPECIAL_KEY_CASE(DELETE,       "Delete");
            SPECIAL_KEY_CASE(RIGHT_ARROW,  "Right Arrow");
            SPECIAL_KEY_CASE(LEFT_ARROW,   "Left Arrow");
            SPECIAL_KEY_CASE(DOWN_ARROW,   "Down Arrow");
            SPECIAL_KEY_CASE(UP_ARROW,     "Up Arrow");
            SPECIAL_KEY_CASE(PAGE_UP,      "Page Up");
            SPECIAL_KEY_CASE(PAGE_DOWN,    "Page Down");
            SPECIAL_KEY_CASE(HOME,         "Home");
            SPECIAL_KEY_CASE(END,          "End");
            SPECIAL_KEY_CASE(CAPS_LOCK,    "Caps Lock");
            SPECIAL_KEY_CASE(SCROLL_LOCK,  "Scroll Lock");
            SPECIAL_KEY_CASE(NUM_LOCK,     "Num Lock");
            SPECIAL_KEY_CASE(PRINT_SCREEN, "Print Screen");
            SPECIAL_KEY_CASE(PAUSE,        "Pause");
            SPECIAL_KEY_CASE(LSHIFT,       "Left Shift");
            SPECIAL_KEY_CASE(LCONTROL,     "Left Control");
            SPECIAL_KEY_CASE(LALT,         "Left Alt");    
            SPECIAL_KEY_CASE(LSUPER,       "Left Super");  
            SPECIAL_KEY_CASE(RSHIFT,       "Right Shift");  
            SPECIAL_KEY_CASE(RCONTROL,     "Right Control");
            SPECIAL_KEY_CASE(RALT,         "Right Alt");    
            SPECIAL_KEY_CASE(RSUPER,       "Right Super");  
            SPECIAL_KEY_CASE(MENU,         "Menu");
            
            SPECIAL_KEY_CASE(NON_US_1, "Non US 1");
            SPECIAL_KEY_CASE(NON_US_2, "Non US 2");

            case Key::UNKNOWN:
            return "ERROR";
        }
        return "ERROR";
#undef BASIC_KEY_CASE
#undef PS_KEY_CASE
#undef SPECIAL_KEY_CASE
    }
}
