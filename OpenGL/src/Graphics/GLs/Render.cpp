#include "Render.h"

namespace Graphics::Render {
    void Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void ClearColorBit() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

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

    void SetRenderWireframe(bool isWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);
    }

    void SetClearColor(const Maths::colorf& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void EnableDepth() {
        glEnable(GL_DEPTH_TEST);
    }

    void DisableDepth() {
        glDisable(GL_DEPTH_TEST);
    }
}
