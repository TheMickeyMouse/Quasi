#pragma once
#include "Mesh.h"
#include "Test.h"
#include "Graphics/Utils/Fonts/Font.h"

namespace Test {
    class TestFontRender : Test {
    public:
        struct Vertex {
            Maths::fvec3  Position;
            Maths::colorf Color;
            Maths::fvec2  TextureCoordinate;
    
            GL_VERTEX_T(Vertex);
            GL_VERTEX_FIELD((Position)(Color)(TextureCoordinate));
            GL_VERTEX_TRANSFORM_FIELDS((Position))
        };
    private:
        Graphics::RenderObject<Vertex> render;
        Graphics::Mesh<Vertex> meshStr, meshAtlas;
        bool showAtlas = false;

        std::string string = " Hello, World!";

        Graphics::Font font;
        Maths::colorf color = 1;

        Maths::mat3D projection = Maths::mat3D::ortho_projection(-320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f);
        Maths::fvec3 modelTranslation = 0;
        Maths::fvec3 modelScale = 1;
        Maths::fvec3 modelRotation = 0;
    public:
        TestFontRender() {}
        ~TestFontRender() override {}

        OPENGL_API void OnInit(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        OPENGL_API void OnDestroy(Graphics::GraphicsDevice& gdevice) override;
    };
}
