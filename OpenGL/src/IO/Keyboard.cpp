#include <algorithm>
#include <GLFW/glfw3.h>

#include "GraphicsDevice.h"

#include "Keyboard.h"

#include "imgui_impl_glfw.h"

namespace IO {
    GLFWwindow* KeyboardType::inputWindow() { return io->gdevice->GetWindow(); }
    const GLFWwindow* KeyboardType::inputWindow() const { return io->gdevice->GetWindow(); }
    
    KeyboardType::KeyboardType(IO& io) : io(io) {
        glfwSetKeyCallback(inputWindow(),
            // clever hack >:)
            [](GLFWwindow* win, auto... args) {
                IO::GetIOPtr(win)->Keyboard.OnGlfwKeyCallback(win, args...);
            } );
    }

    void KeyboardType::Update() {
        prevKeySet = currKeySet;
        while (!queuedKeys.empty()) {
            const KeyIndex ki = queuedKeys.front();
            setKeyStatusOf(currKeySet, ki & 127, ki & 128);
            queuedKeys.pop();
        }
    }

    bool KeyboardType::getKeyStatusOf(const Keyset& ks, KeyIndex ki)     { return ks[ki / 64ULL] & 1ULL << (ki & 63); }
    void KeyboardType::setKeyStatusOf(Keyset& ks, KeyIndex ki, bool val) { ks[ki / 64ULL] = (ks[ki / 64ULL] & ~(1ULL << (ki & 63))) | (uint64)val << (ki & 63); }

    void KeyboardType::OnGlfwKeyCallback(GLFWwindow* window, int key, int positionCode, int action, int modifierBits) {
        ImGui_ImplGlfw_KeyCallback(window, key, positionCode, action, modifierBits);
        const bool isPress = action != GLFW_RELEASE;
        queuedKeys.push((uchar)((isPress << 7) | ToKeyIndex((Key)key)));

    }

    bool KeyboardType::KeyPressed(Key key)   const { return getCurrKeyStatus(key); }
    bool KeyboardType::KeyOnPress(Key key)   const { return !getPrevKeyStatus(key) && getCurrKeyStatus(key); }
    bool KeyboardType::KeyOnRelease(Key key) const { return !getCurrKeyStatus(key) && getPrevKeyStatus(key); }
    bool KeyboardType::AnyPressed()  const { return std::ranges::any_of (currKeySet, [](auto x){ return x != 0; }); }
    bool KeyboardType::NonePressed() const { return std::ranges::none_of(currKeySet, [](auto x){ return x != 0; }); }
    
    std::vector<Key> KeyboardType::KeysPressed() const {
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

    bool KeyboardType::IsValidKey(Key key) {
        return ToKeyIndex(key) != 255;
        // idk man prob not the best but whatev
    }

    // this is the worst code ive ever written
    // but i dont see any other option
    KeyIndex KeyboardType::ToKeyIndex(Key key) {
        using enum Key;
        int offset = 0;
#define RANGE_MATCH(S, E) if ((S) <= key && key <= (E)) return (int)(key - (S)) + offset; offset += (int)(((E) - (S)) + 1)
        
        // chars
        RANGE_MATCH(A, Z);
        // nums
        RANGE_MATCH(NUM_0, NUM_9);
        
        // puncs
        RANGE_MATCH(SPACE, QUOTES);
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

    Key KeyboardType::FromKeyIndex(KeyIndex ki) {
        using enum Key;

#define RANGE_MATCH(S, E) if (ToKeyIndex((S)) <= ki && ki <= ToKeyIndex((E))) return ki - ToKeyIndex((S)) + (S)

        // chars
        RANGE_MATCH(A, Z);

        // nums
        RANGE_MATCH(NUM_0, NUM_9);

        // puncs
        RANGE_MATCH(SPACE, QUOTES);
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

    Key KeyboardType::FromModBits(ModifierKey mod) {
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

    KeyIndex KeyboardType::IndexFromModBits(ModifierKey mod) {
        return ToKeyIndex(FromModBits(mod));
    }
    
    const char* KeyboardType::KeyToStr(Key key) {
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
            SPECIAL_KEY_CASE(QUOTES,               "'");
            SPECIAL_KEY_CASE(COMMA,                ",");
            SPECIAL_KEY_CASE(MINUS,                "-");
            SPECIAL_KEY_CASE(PERIOD,               ".");
            SPECIAL_KEY_CASE(SLASH,                "/");
            SPECIAL_KEY_CASE(SEMICOLON,            ":");
            SPECIAL_KEY_CASE(EQUAL,                "=");
            SPECIAL_KEY_CASE(OPEN_SQUARE_BRACKET,  "[");
            SPECIAL_KEY_CASE(BACKSLASH,            "\\");
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
