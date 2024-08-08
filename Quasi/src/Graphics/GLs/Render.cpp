#include "Render.h"
#include <glp.h>
#include "GLDebug.h"

namespace Quasi::Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        Q_GL_CALL(GL::DrawElements(GL::TRIANGLES, (int)indexBuff.GetUsedLength(), GL::UNSIGNED_INT, nullptr));
    }

    void DrawInstanced(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader, int instances) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        Q_GL_CALL(GL::DrawElementsInstanced(GL::TRIANGLES, (int)indexBuff.GetUsedLength(), GL::UNSIGNED_INT, nullptr, instances));
    }

    void Clear(const BufferBit bit) {
        Q_GL_CALL(GL::Clear((int)bit));
    }

    void SetRenderMode(const RenderMode mode) {
        Q_GL_CALL(GL::PolygonMode(GL::FRONT_AND_BACK, (uint)mode));
    }

    void SetPointSize(float size) {
        Q_GL_CALL(GL::PointSize(size));
    }

    void SetClearColor(const Math::fColor& color) {
        Q_GL_CALL(GL::ClearColor(color.r, color.g, color.b, color.a));
    }

    void Enable(const Capability cap) {
        Q_GL_CALL(GL::Enable((int)cap));
    }

    void Disable(const Capability cap) {
        Q_GL_CALL(GL::Disable((int)cap));
    }

    void UseDepthFunc(const CmpOperation op) {
        Q_GL_CALL(GL::DepthFunc((int)op));
    }

    void UseStencilTest(const CmpOperation op, const int ref, const int mask) {
        Q_GL_CALL(GL::StencilFunc((int)op, ref, mask));
    }

    void UseStencilWriteMask(const int mask) {
        Q_GL_CALL(GL::StencilMask(mask));
    }

    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
        Q_GL_CALL(GL::StencilOp((int)stencilFail, (int)depthFail, (int)pass));
    }

    void UseAlphaFunc(const CmpOperation op, const float ref) {
        Q_GL_CALL(GL::AlphaFunc((int)op, ref));
    }

    void UseBlendConstColor(const Math::fColor& ref) {
        Q_GL_CALL(GL::BlendColor(ref.r, ref.g, ref.b, ref.a));
    }

    void UseBlendFunc(const BlendFactor src, const BlendFactor dest) {
        Q_GL_CALL(GL::BlendFunc((int)src, (int)dest));
    }

    void SetCullFace(FacingMode facing) {
        Q_GL_CALL(GL::CullFace((int)facing));
    }

    void SetFrontFacing(OrientationMode orientation) {
        Q_GL_CALL(GL::FrontFace((int)orientation));
    }

    void SetColorWrite(BufferMode mode) {
        Q_GL_CALL(GL::DrawBuffer((int)mode));
    }
}
