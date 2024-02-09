#pragma once
#include "GLObject.h"
#include "opengl.h"
#include "NumTypes.h"

namespace Graphics {
	class VertexBuffer : public GLObject<BufferHandler<BufferType::VERTEX>> {
	public:
		OPENGL_API VertexBuffer(const void* data, uint size);
	};
}
