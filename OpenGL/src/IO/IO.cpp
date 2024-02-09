#include "IO.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "TimeType.h"

namespace IO {
    void Init(Graphics::GraphicsDevice& gd) {
        Mouse = MouseType { gd };
        Keyboard = KeyboardType { gd };
    }

    void Update() {
        Mouse.Update();
        Keyboard.Update();
        Time.Update();
    }
}
