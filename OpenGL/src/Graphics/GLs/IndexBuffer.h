#pragma once
#include "GLObject.h"
#include "NumTypes.h"
#include "opengl.h"

namespace Graphics {
	class IndexBuffer : public GLObject<BufferHandler<BufferType::INDEX>> {
	private:
		uint length = 0;
	public:
		OPENGL_API IndexBuffer(const void* data, uint length);

		[[nodiscard]] uint GetLength() const { return length; }
	};
}
