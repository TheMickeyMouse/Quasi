#pragma once
#include "GLObject.h"
#include "NumTypes.h"
#include <core.h>

namespace Graphics {
	class IndexBuffer : public GLObject<BufferHandler<BufferType::INDEX>> {
	private:
		uint length = 0;
	public:
		OPENGL_API IndexBuffer(std::span<const uint> data);

		[[nodiscard]] uint GetLength() const { return length; }
	};
}
