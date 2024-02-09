#pragma once
#include "NumTypes.h"
#include "opengl.h"

namespace Graphics {
	class IndexBuffer {
		private:
			glID rendererID = GL_NULL;
			uint length = 0;
		public:
			OPENGL_API IndexBuffer(const void* data, uint length);
			OPENGL_API ~IndexBuffer();

			OPENGL_API void Bind() const;
			OPENGL_API void Unbind() const;

			[[nodiscard]] uint GetLength() const { return length; }
	};
}
