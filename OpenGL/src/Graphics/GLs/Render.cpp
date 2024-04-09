#include "Render.h"
#include <GL/glew.h>
#include "GLDebug.h"

namespace Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GL_CALL(glDrawElements(GL_TRIANGLES, (int)indexBuff.GetLength(), (int)GLTypeID::UINT, nullptr));
    }

    void Draw(const VertexArray& vertexArr, const DynamicIndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GL_CALL(glDrawElements(GL_TRIANGLES, (int)indexBuff.GetUsedLength(), (int)GLTypeID::UINT, nullptr));
    }

    void DrawInstanced(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader, int instances) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)indexBuff.GetLength(), (int)GLTypeID::UINT, nullptr, instances));
    }

    void DrawInstanced(const VertexArray& vertexArr, const DynamicIndexBuffer& indexBuff, const Shader& shader, int instances) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)indexBuff.GetUsedLength(), (int)GLTypeID::UINT, nullptr, instances));
    }

    void Clear(const BufferBit bit) {
        GL_CALL(glClear((int)bit));
    }

    void SetRenderMode(const RenderMode mode) {
        GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, (uint)mode));
    }

    void SetPointSize(float size) {
        GL_CALL(glPointSize(size));
    }

    void SetClearColor(const Maths::colorf& color) {
        GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void Enable(const Capability cap) {
        GL_CALL(glEnable((int)cap));
    }

    void Disable(const Capability cap) {
        GL_CALL(glDisable((int)cap));
    }

    void UseDepthFunc(const CmpOperation op) {
        GL_CALL(glDepthFunc((int)op));
    }

    void UseStencilTest(const CmpOperation op, const int ref, const int mask) {
        GL_CALL(glStencilFunc((int)op, ref, mask));
    }

    void UseStencilWriteMask(const int mask) {
        GL_CALL(glStencilMask(mask));
    }

    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
        GL_CALL(glStencilOp((int)stencilFail, (int)depthFail, (int)pass));
    }

    void UseAlphaFunc(const CmpOperation op, const float ref) {
        GL_CALL(glAlphaFunc((int)op, ref));
    }

    void UseBlendConstColor(const Maths::colorf& ref) {
        GL_CALL(glBlendColor(ref.r, ref.g, ref.b, ref.a));
    }

    void UseBlendFunc(const BlendFactor src, const BlendFactor dest) {
        GL_CALL(glBlendFunc((int)src, (int)dest));
    }

    void SetCullFace(FacingMode facing) {
        GL_CALL(glCullFace((int)facing));
    }

    void SetFrontFacing(OrientationMode orientation) {
        GL_CALL(glFrontFace((int)orientation));
    }

    void SetColorWrite(BufferMode mode) {
        GL_CALL(glDrawBuffer((int)mode));
    }
}
