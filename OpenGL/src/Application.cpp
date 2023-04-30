#define TEST_MODE 0
#define MEM_DIAGNOSE 0

#if TEST_MODE == 1
#include "GraphicsDevice.h"
#include "TestingFramework.h"
#else
#include "Game/Game.h"
#endif

#if MEM_DIAGNOSE == 1
static size_t MD_MemoryUsed = 0;
static size_t MD_GameInitMemoryUsed = 0;
static bool MD_IsGameLoop = false;

void* operator new(size_t size) {
    if (MD_IsGameLoop) {
        // some helper if's
    }
    MD_MemoryUsed += size;
    return malloc(size);
}

void operator delete(void* mem, size_t size) {
    if (MD_IsGameLoop) {
        // some helper if's
    }
    MD_MemoryUsed -= size;
    free(mem);
}
#endif

int main() {
#if TEST_MODE == 1
    {
        std::unique_ptr<Graphics::GraphicsDevice> GraphicsDevice = Graphics::GraphicsDevice::Initialize();
        Test::TestManager testm;
        // renderer.SetRenderMode('w');
#if MEM_DIAGNOSE == 1
        MD_GameInitMemoryUsed = MD_MemoryUsed;
        MD_MemoryUsed = 0;

        MD_IsGameLoop = true;
#endif
    
        /* Loop until the user closes the window */
        while (GraphicsDevice->WindowIsOpen()) {
            testm.OnUpdate();
        
            GraphicsDevice->BeginRender();
        
            testm.OnImGuiRender();
            testm.OnRender(GraphicsDevice->GetRenderer());

            GraphicsDevice->EndRender();

#if MEM_DIAGNOSE == 1
            if (MD_MemoryUsed != 0) {
                // error            
            }
#endif
        }
#if MEM_DIAGNOSE == 1
        MD_MemoryUsed += MD_GameInitMemoryUsed;
    }
    if (MD_MemoryUsed != 0) {
        // error
    }
#else
    }
#endif
    return 0;
#else
    {
        Game::Game game{};
        /* Loop until the user closes the window */
#if MEM_DIAGNOSE == 1
        MD_GameInitMemoryUsed = MD_MemoryUsed;
        MD_MemoryUsed = 0;

        MD_IsGameLoop = true;
#endif
        while (game.IsWindowOpen()) {
            game.Update();
        
            game.Render();

#if MEM_DIAGNOSE == 1
            if (MD_MemoryUsed != 0) {
                // error            
            }
#endif
        }
#if MEM_DIAGNOSE == 1
        MD_MemoryUsed += MD_GameInitMemoryUsed;
    }
    if (MD_MemoryUsed != 0) {
        // error
    }
#else
    }
#endif
    return 0;
#endif
}
