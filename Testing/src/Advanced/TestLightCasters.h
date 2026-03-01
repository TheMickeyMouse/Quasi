#pragma once
#include "CameraController3D.h"
#include "Light.h"
#include "Test.h"
#include "Effects/Bloom.h"
#include "ModelLoading/OBJModel.h"

namespace Quasi::Graphics {
    struct MTLMaterial;
}

namespace Test {
    class TestLightCasters : public Test {
    public:
        struct Vertex {
            Math::fv3 Position, Normal;
            int MaterialID;

            QuasiDefineVertex$(Vertex, 3D, (Position, Position)(Normal, Normal)(MaterialID));
        };
    private:
        Graphics::RenderObject<Vertex> scene;
        Vec<Graphics::MTLMaterial> materials;
        Graphics::Mesh<Vertex> worldMesh;
        Vec<Graphics::Light> lights;
        static constexpr int MAX_LIGHTS = 8;
        Graphics::CameraController3D camera;
        Graphics::Bloom bloom {{ 1920, 1080 }};

        Graphics::RenderObject<Graphics::Vertex3D> lightScene;
        Graphics::Mesh<Graphics::Vertex3D> lightBox;

        float ambientStrength = 0.03f, specularStrength = 1.2f;

        int mipLod = 0;

        DEFINE_TEST_T(TestLightCasters, ADVANCED)
    public:
        TestLightCasters() = default;

        void OnInit(Graphics::GraphicsDevice& gdevice) override;
        void OnUpdate(Graphics::GraphicsDevice& gdevice, float deltaTime) override;
        void OnRender(Graphics::GraphicsDevice& gdevice) override;
        void OnImGuiRender(Graphics::GraphicsDevice& gdevice) override;
        void OnDestroy(Graphics::GraphicsDevice& gdevice) override;

        void UniformMaterial(const String& name, const Graphics::MTLMaterial& material);
        void UniformLight(const String& name, const Graphics::Light& light);

        void AddPointLight(const Graphics::PointLight& point, const Math::fColor& color);
    };
}
