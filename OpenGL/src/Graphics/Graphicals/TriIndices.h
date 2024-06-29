#pragma once

namespace Quasi::Graphics {
	struct TriIndices {
		u32 a, b, c;

		[[nodiscard]] TriIndices add(u32 i) const { return { a + i, b + i, c + i }; }
		TriIndices& incr(u32 i) { a += i; b += i; c += i; return *this; }

		TriIndices  operator+ (std::convertible_to<u32> auto i) const { return add((u32)i); }
		TriIndices& operator+=(std::convertible_to<u32> auto i) { return incr((u32)i); }
	};
}