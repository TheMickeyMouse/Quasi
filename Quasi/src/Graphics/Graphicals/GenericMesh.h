#pragma once
#include <utility>

#include <span>

#include "Ref.h"
#include "Utils/Type.h"

namespace Quasi::Graphics {
	class RenderData;
	template <class T> class Mesh;

	class GenericMesh {
		// used so that 'singlebyte' is a custom defined type, unlike char
		struct singlebyte { char _; };
		Ref<Mesh<singlebyte>> gmesh; // using the fact that mesh<T> is always 104 bytes
	public:
		GenericMesh(void* m) : gmesh(DerefPtr((Mesh<singlebyte>*)m)) {}

		template <class T> static GenericMesh From(Mesh<T>* m) {
			return GenericMesh { (Mesh<singlebyte>*)m };
		}

		template <class T> Mesh<T>& As() {
			return gmesh.TransmuteAs<Mesh<T>>();
		}

		template <class T> const Mesh<T>& As() const {
			return gmesh.TransmuteAs<Mesh<T>>();
		}

		void Set(void* mesh);
		template <class T> void Set(Mesh<T>* mesh) {
			Set((void*)mesh);
		}

		[[nodiscard]] usize vSizeBytes() const;
		[[nodiscard]] Span<const byte> vDataBytes() const;
		[[nodiscard]] usize iSize() const;
	};
}
