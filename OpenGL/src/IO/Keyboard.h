#pragma once
#include <array>
#include <queue>
#include <GLFW/glfw3.h>
#include <core.h>

#include "NumTypes.h"
#include "stdu/enum_utils.h"
#include "stdu/ref.h"

struct GLFWwindow;

namespace Graphics {
    class GraphicsDevice;
}

namespace IO {
    #define KEY_IMPL_GLFW(K, NAME_PRE, GLFW_PRE) NAME_PRE##K = GLFW_KEY_##GLFW_PRE##K
    // DO NOT CHANGE ORDER!
    enum class Key : int {
        UNKNOWN = -1,

        // CHARACTER
        KEY_IMPL_GLFW(A, , ),
        B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // NUMBER
        KEY_IMPL_GLFW(0, NUM_, ),
        NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,

        // KEYPAD
        KEY_IMPL_GLFW(0, KEYPAD_, KP_),
        KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_7, KEYPAD_8, KEYPAD_9,
        KEYPAD_DECIMAL, KEYPAD_DIVIDE, KEYPAD_MULTIPLY, KEYPAD_SUBTRACT, KEYPAD_ADD, KEYPAD_ENTER, KEYPAD_EQUAL,

        // SYMBOLS
        KEY_IMPL_GLFW(SPACE, , ),
        KEY_IMPL_GLFW(, QUOTATIONS, APOSTROPHE),
        KEY_IMPL_GLFW(COMMA, , ),
        KEY_IMPL_GLFW(MINUS, , ),
        KEY_IMPL_GLFW(PERIOD, , ),
        KEY_IMPL_GLFW(SLASH, , ),
        KEY_IMPL_GLFW(SEMICOLON, , ),
        KEY_IMPL_GLFW(EQUAL, , ),
        KEY_IMPL_GLFW(, OPEN_SQUARE_BRACKET, LEFT_BRACKET),
        KEY_IMPL_GLFW(, BACK_SLASH, BACKSLASH),
        KEY_IMPL_GLFW(, CLOSE_SQUARE_BRACKET, RIGHT_BRACKET),
        KEY_IMPL_GLFW(GRAVE_ACCENT, , ),

        // FUNCTION
        KEY_IMPL_GLFW(F1, , ),
        F2, F3, F4, F5, F6, F7, F8, F9, F10,
        F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

        // UTILITY
        KEY_IMPL_GLFW(ESCAPE, , ),
        ENTER, TAB, BACKSPACE, INSERT, DELETE,
        RIGHT_ARROW, LEFT_ARROW, DOWN_ARROW, UP_ARROW,
        PAGE_UP, PAGE_DOWN,
        HOME, END,
        KEY_IMPL_GLFW(CAPS_LOCK, , ),
        SCROLL_LOCK, NUM_LOCK,
        PRINT_SCREEN, PAUSE,
        KEY_IMPL_GLFW(SHIFT, L, LEFT_),
        LCONTROL, LALT, LSUPER,
        RSHIFT, RCONTROL, RALT, RSUPER,
        MENU,

        // OTHER
        KEY_IMPL_GLFW(1, NON_US_, WORLD_),
        KEY_IMPL_GLFW(2, NON_US_, WORLD_),

        LAST = MENU
    };
    STDU_IMPL_ENUM_OPERATORS(Key);
    #undef KEY_IMPL_GLFW

    #define MOD_IMPL_GLFW(K) K = GLFW_MOD_##K
    enum class ModifierKey {
        MOD_IMPL_GLFW(SHIFT),
        MOD_IMPL_GLFW(CONTROL),
        MOD_IMPL_GLFW(ALT),
        MOD_IMPL_GLFW(SUPER),
        MOD_IMPL_GLFW(CAPS_LOCK),
        MOD_IMPL_GLFW(NUM_LOCK),
    };
    #undef MOD_IMPL_GLFW
    STDU_IMPL_ENUM_OPERATORS(ModifierKey);

    // TODO: this v
    // enum class KeyType : int {
    //     ANY       = 127,
    //     CHARACTER = 1,  // A ~ Z
    //     NUMBER    = 2,  // 0 ~ 9
    //     KEYPAD    = 4,  // keypads
    //     SYMBOL    = 8,  // symbols
    //     FUNCTION  = 16, // fx keys
    //     UTILITY   = 32, // ctrl, shift
    //     OTHER     = 64, // other
    // };

    using KeyIndex = uchar;

    class IO;

    struct KeyboardType {
        OPENGL_API explicit KeyboardType(IO& io);
        explicit KeyboardType(std::nullptr_t) {}

        OPENGL_API static bool IsValidKey(Key key);
        // ordering: alphabet chars -> nums -> punc -> keypad -> func -> util -> other
        // i want this to be constexpr but whatev
        OPENGL_API static KeyIndex ToKeyIndex(Key key);
        OPENGL_API static Key FromKeyIndex(KeyIndex ki);
        OPENGL_API static Key FromModBits(ModifierKey mod);
        OPENGL_API static KeyIndex IndexFromModBits(ModifierKey mod);

        inline static constexpr KeyIndex MAX_KEY_INDEX = 119; // ToKeyIndex(Key::NON_US_2); // should be 119
        inline static constexpr KeyIndex KEY_COUNT = MAX_KEY_INDEX + 1; // MAX_KEY_INDEX + 1; // should be 120
        inline static constexpr KeyIndex KEYSET_SIZE = (KEY_COUNT - 1) / 64 + 1; // TODO: MAKE MODIF KEYS WORK

        void Update();

        OPENGL_API [[nodiscard]] bool KeyPressed(Key key) const;
        OPENGL_API [[nodiscard]] bool KeyOnPress(Key key) const;
        OPENGL_API [[nodiscard]] bool KeyOnRelease(Key key) const;
        OPENGL_API [[nodiscard]] bool AnyPressed() const;
        OPENGL_API [[nodiscard]] bool NonePressed() const;
        OPENGL_API [[nodiscard]] std::vector<Key> KeysPressed() const;

        static const char* KeyToStr(Key key);

        stdu::ref<IO> io;
    private:
        using Keyset = std::array<uint64, KEYSET_SIZE>;
        Keyset currKeySet = {};
        Keyset prevKeySet = {};
        std::queue<KeyIndex> queuedKeys = {};

        OPENGL_API GLFWwindow* inputWindow();
        OPENGL_API [[nodiscard]] const GLFWwindow* inputWindow() const;

        [[nodiscard]] bool getCurrKeyStatus(KeyIndex i) const { return getKeyStatusOf(currKeySet, i); }
        [[nodiscard]] bool getPrevKeyStatus(KeyIndex i) const { return getKeyStatusOf(prevKeySet, i); }
        [[nodiscard]] bool getCurrKeyStatus(Key k) const { return getKeyStatusOf(currKeySet, ToKeyIndex(k)); }
        [[nodiscard]] bool getPrevKeyStatus(Key k) const { return getKeyStatusOf(prevKeySet, ToKeyIndex(k)); }

        OPENGL_API static bool getKeyStatusOf(const Keyset& ks, KeyIndex ki);
        OPENGL_API static void setKeyStatusOf(Keyset& ks, KeyIndex ki, bool val);

        OPENGL_API void OnGlfwKeyCallback(GLFWwindow* window, int key, int positionCode, int action, int modifierBits);
    };
}
