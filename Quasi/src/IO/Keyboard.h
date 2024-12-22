#pragma once
#include <bitset>
#include <queue>

#include "Utils/Ref.h"

struct GLFWwindow;

namespace Quasi::Graphics {
    class GraphicsDevice;
}

namespace Quasi::IO {
    // DO NOT CHANGE ORDER!
    enum Key {
        UNKNOWN = -1,

        // CHARACTER
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // NUMBER
        NUM_0 = 48, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9,

        // KEYPAD
        KEYPAD_0 = 320, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_7, KEYPAD_8, KEYPAD_9,
        KEYPAD_DECIMAL, KEYPAD_DIVIDE, KEYPAD_MULTIPLY, KEYPAD_SUBTRACT, KEYPAD_ADD, KEYPAD_ENTER, KEYPAD_EQUAL,

        // SYMBOLS
        SPACE  = 32,
        QUOTES = 39,  // '
        COMMA  = 44,  // ,
        MINUS  = 45,  // -
        PERIOD = 46,  // .
        SLASH  = 47,  // /
        SEMICOLON = 59, // ;
        EQUAL     = 61, // =
        OPEN_SQUARE_BRACKET  = 91,  // [
        BACKSLASH            = 92,
        CLOSE_SQUARE_BRACKET = 93,  // ]
        GRAVE_ACCENT  = 96,  // `

        // FUNCTION
        F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10,
        F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

        // UTILITY
        ESCAPE = 256,
        ENTER, TAB, BACKSPACE, INSERT, DELETE,
        RIGHT_ARROW, LEFT_ARROW, DOWN_ARROW, UP_ARROW,
        PAGE_UP, PAGE_DOWN,
        HOME, END,

        CAPS_LOCK = 280,
        SCROLL_LOCK, NUM_LOCK,
        PRINT_SCREEN, PAUSE,

        LSHIFT = 340,
        LCONTROL, LALT, LSUPER,
        RSHIFT, RCONTROL, RALT, RSUPER,
        MENU,

        // OTHER
        NON_US_1 = 161,
        NON_US_2 = 162,

        LAST = MENU
    };

    enum ModifierKey {
        M_SHIFT     = 1 << 0,
        M_CONTROL   = 1 << 1,
        M_ALT       = 1 << 2,
        M_SUPER     = 1 << 3,
        M_CAPS_LOCK = 1 << 4,
        M_NUM_LOCK  = 1 << 5,
    };

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

    using KeyIndex = byte;

    class IO;

    struct KeyboardType {
        explicit KeyboardType(IO& io);

        static bool IsValidKey(Key key);
        // ordering: alphabet chars -> nums -> punc -> keypad -> func -> util -> other
        // i want this to be constexpr but whatev
        static KeyIndex ToKeyIndex(Key key);
        static Key FromKeyIndex(KeyIndex ki);
        static Key FromModBits(ModifierKey mod);
        static KeyIndex IndexFromModBits(ModifierKey mod);

        static constexpr KeyIndex MAX_KEY_INDEX = 119; // ToKeyIndex(Key::NON_US_2); // should be 119
        static constexpr KeyIndex KEY_COUNT = MAX_KEY_INDEX + 1; // MAX_KEY_INDEX + 1; // should be 120
        // TODO: MAKE MODIF KEYS WORK

        void Update();

        bool KeyPressed(Key key) const;
        bool KeyOnPress(Key key) const;
        bool KeyOnRelease(Key key) const;
        bool AnyPressed() const;
        bool NonePressed() const;
        void VisitKeysPressed(FuncRef<void(Key k)> callback) const;

        static Str KeyToStr(Key key);

        Ref<IO> io;
    private:
        using Keyset = std::bitset<KEY_COUNT>;
        Keyset currKeySet = {};
        Keyset prevKeySet = {};
        std::queue<KeyIndex> queuedKeys = {};

        GLFWwindow* inputWindow();
        const GLFWwindow* inputWindow() const;

        bool getCurrKeyStatus(KeyIndex i) const { return currKeySet[i]; }
        bool getPrevKeyStatus(KeyIndex i) const { return prevKeySet[i]; }
        bool getCurrKeyStatus(Key k) const { return currKeySet[ToKeyIndex(k)]; }
        bool getPrevKeyStatus(Key k) const { return prevKeySet[ToKeyIndex(k)]; }

        void OnGlfwKeyCallback(GLFWwindow* window, int key, int positionCode, int action, int modifierBits);
    };
}
