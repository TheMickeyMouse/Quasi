#include "Graphics/Graphicals/GraphicsDevice.h"
#include "Font.h"
#include "test/TestingFramework.h"
// #include "Mesh.h"
// #include "imgui.h"
// #include "Keyboard.h"
// #include "Mouse.h"

// #include <iostream>
// #include "rich_string.h"

int main() {
    {
        Graphics::GraphicsDevice GraphicsDevice = Graphics::GraphicsDevice::Initialize({ 1200, 900 });
        Test::TestManager testm;

        testm.OnInit(GraphicsDevice);
        while (GraphicsDevice.WindowIsOpen()) {
            testm.OnUpdate();

            GraphicsDevice.BeginRender();

            testm.OnRender(GraphicsDevice);
            testm.OnImGuiRender(GraphicsDevice);

            GraphicsDevice.EndRender();
        }
        testm.OnDestroy(GraphicsDevice);
    }
    // using namespace Maths;
    // using namespace IO;
    // fvec3 trans = { 0, 0, -2 }, scale = 1, rot;
    // float speed = 0.025f, sensitivity = 0.01f;
    // bool isLocked = true;
    //
    // Mouse.Lock();
    //
    // Graphics::Mesh<VertexColor3D> cube = Graphics::MeshUtils::CubeMesh(0, 1, 1, 1);
    // Graphics::RenderObject render = GraphicsDevice.CreateNewRender<VertexColor3D>(); 
    // render.BindMeshes(&cube, 1);
    //
    // render.SetProjection(mat3D::perspective_fov(90.0f, 4.0f / 3, 0.1f, 100));
    // render.UseShaderFromFile("basic_tex.glsl");
    // // GraphicsDevice->RegisterMeshes();
    // // * Loop until the user closes the window */
    // while (GraphicsDevice.WindowIsOpen()) {
    //     GraphicsDevice.BeginRender();
    //
    //     mat3D rotation = mat3D::rotate_identity({ 0, rot.y, 0 });
    //     if (Keyboard.KeyPressed(Key::W))      trans += rotation * fvec3::FRONT() * -speed;
    //     if (Keyboard.KeyPressed(Key::S))      trans += rotation * fvec3::BACK()  * -speed;
    //     if (Keyboard.KeyPressed(Key::D))      trans += rotation * fvec3::LEFT()  *  speed;
    //     if (Keyboard.KeyPressed(Key::A))      trans += rotation * fvec3::RIGHT() *  speed;
    //     if (Keyboard.KeyPressed(Key::SPACE))  trans += fvec3::UP()    * -speed;
    //     if (Keyboard.KeyPressed(Key::LSHIFT)) trans += fvec3::DOWN()  * -speed;
    //
    //     if(Keyboard.KeyOnPress(Key::ESCAPE)) {
    //         isLocked ^= true;
    //         if (isLocked)
    //             Mouse.Lock();
    //         else
    //             Mouse.Show();
    //     }
    //
    //     if (isLocked) {
    //         rot = Mouse.GetMousePosPx().perpend() * -sensitivity;
    //     }
    //
    //     render.ResetData();
    //     render.Render();
    //
    //     mat3D mat = mat3D::transform(trans, scale, rot);
    //     render.SetCamera(mat.inv());
    //     ImGui::SliderFloat3("Rot", rot.begin(),   -3.14f, 3.14f);
    //     ImGui::SliderFloat3("Sca", scale.begin(),  0,     3    );
    //     ImGui::SliderFloat3("Tra", trans.begin(), -10,    10   );
    //     ImGui::SliderFloat("Speed", &speed, 0, 1);
    //     ImGui::SliderFloat("Sensitivity", &sensitivity, 0, 1);
    //
    //     GraphicsDevice.DebugMenu();
    //
    //     GraphicsDevice.EndRender();
    // }

//     using namespace stdu;
//     const rich_string md =
//         R"(`Features include: <br>
// New Line with `<br>`<br>
// **bold text** with `**`<br>
// *italic text* with `*`<br>
// ***bold & italic*** with `***``<br>
// ~~Strike trough like dis~~ with `~~`)"_md;
//
//     std::cout << md.debug_rawstr() << '\n';
    
    return 0;
}
