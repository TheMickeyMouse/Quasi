#include "Render.h"
#include <glp.h>
#include "GLDebug.h"
#include "VertexArray.h"
#include "../RenderData.h"

namespace Quasi::Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        QGLCall$(GL::DrawElements(GL::TRIANGLES, (int)indexBuff.GetUsedLength(), GL::UNSIGNED_INT, nullptr));
    }

    void DrawInstanced(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader, int instances) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        QGLCall$(GL::DrawElementsInstanced(GL::TRIANGLES, (int)indexBuff.GetUsedLength(), GL::UNSIGNED_INT, nullptr, instances));
    }

    void Draw(const RenderData& dat, const Shader& s) {
        Draw(dat.varray, dat.ibo, s);
    }

    void Draw(const RenderData& dat) {
        Draw(dat, dat.shader);
    }

    void DrawInstanced(const RenderData& dat, const Shader& s, int instances) {
        DrawInstanced(dat.varray, dat.ibo, s, instances);
    }

    void DrawInstanced(const RenderData& dat, int instances) {
        DrawInstanced(dat, dat.shader, instances);
    }

    void Clear(const BufferBit bit) {
        QGLCall$(GL::Clear((int)bit));
    }

    void SetRenderMode(const RenderMode mode) {
        QGLCall$(GL::PolygonMode(GL::FRONT_AND_BACK, (uint)mode));
    }

    void SetPointSize(float size) {
        QGLCall$(GL::PointSize(size));
    }

    void SetClearColor(const Math::fColor& color) {
        QGLCall$(GL::ClearColor(color.r, color.g, color.b, color.a));
    }

    void Enable(const Capability cap) {
        QGLCall$(GL::Enable((int)cap));
    }

    void Disable(const Capability cap) {
        QGLCall$(GL::Disable((int)cap));
    }

    void UseDepthFunc(const CmpOperation op) {
        QGLCall$(GL::DepthFunc((int)op));
    }

    void UseStencilTest(const CmpOperation op, const int ref, const int mask) {
        QGLCall$(GL::StencilFunc((int)op, ref, mask));
    }

    void UseStencilWriteMask(const int mask) {
        QGLCall$(GL::StencilMask(mask));
    }

    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
        QGLCall$(GL::StencilOp((int)stencilFail, (int)depthFail, (int)pass));
    }

    void UseAlphaFunc(const CmpOperation op, const float ref) {
        QGLCall$(GL::AlphaFunc((int)op, ref));
    }

    void UseBlendConstColor(const Math::fColor& ref) {
        QGLCall$(GL::BlendColor(ref.r, ref.g, ref.b, ref.a));
    }

    void UseBlendFunc(const BlendFactor src, const BlendFactor dest) {
        QGLCall$(GL::BlendFunc((int)src, (int)dest));
    }

    void SetCullFace(FacingMode facing) {
        QGLCall$(GL::CullFace((int)facing));
    }

    void SetFrontFacing(OrientationMode orientation) {
        QGLCall$(GL::FrontFace((int)orientation));
    }

    void SetColorWrite(BufferMode mode) {
        QGLCall$(GL::DrawBuffer((int)mode));
    }
}
