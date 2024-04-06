#pragma once

#include "NumTypes.h"

namespace Graphics {
	struct TriIndices {
		uint a, b, c;

		[[nodiscard]] TriIndices add(uint i) const { return { a + i, b + i, c + i }; }
		TriIndices& incr(uint i) { a += i; b += i; c += i; return *this; }

		TriIndices  operator+(std::convertible_to<uint> auto i) const { return add((uint)i); }
		TriIndices& operator+=(std::convertible_to<uint> auto i) { return incr((uint)i); }
	};
}