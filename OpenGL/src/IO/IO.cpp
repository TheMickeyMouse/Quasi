#include "Mouse.h"
#include "Keyboard.h"
#include "TimeType.h"
#include "IO.h"

namespace IO {
    IO::IO(Graphics::GraphicsDevice& gd) :
        Keyboard(gd),
        Mouse(gd),
        Time() {}

    void IO::Update() {
        Mouse.Update();
        Keyboard.Update();
        Time.Update();
    }
}
