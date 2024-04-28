#pragma once
#include "NumTypes.h"
#include <utility>

#include <span>

namespace Graphics {
	template <class T> class Mesh;

	class GenericMesh {
		// used so that 'singlebyte' is a custom defined type, unlike char
		struct singlebyte { char _; };
		Mesh<singlebyte>* ptr; // using the fact that mesh<T> is always 104 bytes
	public:
		GenericMesh(void* m) : ptr((Mesh<singlebyte>*)m) {}
		~GenericMesh();

		GenericMesh(const GenericMesh&) = delete;
		GenericMesh& operator=(const GenericMesh&) = delete;

		static void Transfer(GenericMesh& dest, GenericMesh&& from);
		GenericMesh(GenericMesh&& mesh) noexcept { Transfer(*this, std::move(mesh)); }
		GenericMesh& operator=(GenericMesh&& mesh) noexcept { Transfer(*this, std::move(mesh)); return *this; }

		template <class T> static GenericMesh From(Mesh<T>* m) {
			return GenericMesh { (Mesh<singlebyte>*)m };
		}

		template <class T> Mesh<T>& As() {
			return *reinterpret_cast<Mesh<T>*>(ptr);
		}

		template <class T> const Mesh<T>& As() const {
			return *reinterpret_cast<const Mesh<T>*>(ptr);
		}

		void Set(void* mesh);
		template <class T> void Set(Mesh<T>* mesh) {
			Set((void*)mesh);
		}

		uint& deviceIndex();
		[[nodiscard]] uint deviceIndex() const;

		[[nodiscard]] usize vSizeBytes() const;
		[[nodiscard]] std::span<const byte> vDataBytes() const;
		[[nodiscard]] usize iSize() const;
	};
}
