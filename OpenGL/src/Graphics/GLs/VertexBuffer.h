#pragma once
#include "GLObject.h"


namespace Graphics {
	class VertexBuffer : public GLObject<BufferHandler<BufferType::VERTEX>> {
	public:
		VertexBuffer(stdu::cbyte_span data);

		friend class GraphicsDevice;
	};
}
