#include "Render.h"

namespace Graphics::Render {
    void Draw(const VertexArray& vertexArr, const IndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GLCALL(glDrawElements(GL_TRIANGLES, indexBuff.GetLength(), (int)GLTypeID::UINT, nullptr));
    }

    void Draw(const VertexArray& vertexArr, const DynamicIndexBuffer& indexBuff, const Shader& shader) {
        vertexArr.Bind();
        indexBuff.Bind();
        shader.Bind();
        GLCALL(glDrawElements(GL_TRIANGLES, indexBuff.GetUsedLength(), (int)GLTypeID::UINT, nullptr));
    }

    void Clear(const BufferBit bit) {
        GLCALL(glClear((int)bit));
    }

    void SetRenderWireframe(const bool isWireframe) {
        GLCALL(glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL));
    }

    void SetClearColor(const Maths::colorf& color) {
        GLCALL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void Enable(const Capability cap) {
        GLCALL(glEnable((int)cap));
    }

    void Disable(const Capability cap) {
        GLCALL(glDisable((int)cap));
    }

    void UseDepthFunc(const CmpOperation op) {
        GLCALL(glDepthFunc((int)op));
    }

    void UseStencilTest(const CmpOperation op, const int ref, const int mask) {
        GLCALL(glStencilFunc((int)op, ref, mask));
    }

    void UseStencilWriteMask(const int mask) {
        GLCALL(glStencilMask(mask));
    }

    void UseStencilWriteOp(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
        GLCALL(glStencilOp((int)stencilFail, (int)depthFail, (int)pass));
    }

    void UseAlphaFunc(const CmpOperation op, const float ref) {
        GLCALL(glAlphaFunc((int)op, ref));
    }

    void UseBlendConstColor(const Maths::colorf& ref) {
        GLCALL(glBlendColor(ref.r, ref.g, ref.b, ref.a));
    }

    void UseBlendFunc(const BlendFactor src, const BlendFactor dest) {
        GLCALL(glBlendFunc((int)src, (int)dest));
    }
}
