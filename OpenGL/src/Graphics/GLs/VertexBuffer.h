#pragma once
#include "GLObject.h"
#include <core.h>

namespace Graphics {
	class VertexBuffer : public GLObject<BufferHandler<BufferType::VERTEX>> {
	public:
		OPENGL_API VertexBuffer(stdu::cbyte_span data);

		friend class GraphicsDevice;
	};
}
