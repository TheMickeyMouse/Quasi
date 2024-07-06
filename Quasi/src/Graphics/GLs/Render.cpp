#include "Render.h"
#include <GL/glew.h>
#include "GLDebug.h"

namespace Quasi::Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        Q_GL_CALL(glDrawElements(GL_TRIANGLES, (int)indexBuff.GetUsedLength(), (int)GLTypeID::UINT, nullptr));
    }

    void DrawInstanced(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader, int instances) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        Q_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)indexBuff.GetUsedLength(), (int)GLTypeID::UINT, nullptr, instances));
    }

    void Clear(const BufferBit bit) {
        Q_GL_CALL(glClear((int)bit));
    }

    void SetRenderMode(const RenderMode mode) {
        Q_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, (uint)mode));
    }

    void SetPointSize(float size) {
        Q_GL_CALL(glPointSize(size));
    }

    void SetClearColor(const Math::fColor& color) {
        Q_GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void Enable(const Capability cap) {
        Q_GL_CALL(glEnable((int)cap));
    }

    void Disable(const Capability cap) {
        Q_GL_CALL(glDisable((int)cap));
    }

    void UseDepthFunc(const CmpOperation op) {
        Q_GL_CALL(glDepthFunc((int)op));
    }

    void UseStencilTest(const CmpOperation op, const int ref, const int mask) {
        Q_GL_CALL(glStencilFunc((int)op, ref, mask));
    }

    void UseStencilWriteMask(const int mask) {
        Q_GL_CALL(glStencilMask(mask));
    }

    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
        Q_GL_CALL(glStencilOp((int)stencilFail, (int)depthFail, (int)pass));
    }

    void UseAlphaFunc(const CmpOperation op, const float ref) {
        Q_GL_CALL(glAlphaFunc((int)op, ref));
    }

    void UseBlendConstColor(const Math::fColor& ref) {
        Q_GL_CALL(glBlendColor(ref.r, ref.g, ref.b, ref.a));
    }

    void UseBlendFunc(const BlendFactor src, const BlendFactor dest) {
        Q_GL_CALL(glBlendFunc((int)src, (int)dest));
    }

    void SetCullFace(FacingMode facing) {
        Q_GL_CALL(glCullFace((int)facing));
    }

    void SetFrontFacing(OrientationMode orientation) {
        Q_GL_CALL(glFrontFace((int)orientation));
    }

    void SetColorWrite(BufferMode mode) {
        Q_GL_CALL(glDrawBuffer((int)mode));
    }
}
