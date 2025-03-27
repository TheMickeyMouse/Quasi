#pragma once
#include "Numeric.h"
#include "Span.h"
#include "Option.h"
#include "Ref.h"

namespace Quasi::Hashing {
    enum Hash : usize {};

    inline Hash EmptyHash() { return Hash {}; }
    Hash  AsHash(usize x);
    usize AsIndex(Hash h);

    Hash HashInt(usize x);
    Hash HashBytes(Span<const byte> bytes);
    Hash HashCombine(Hash a, Hash b);

    template <class T>
    Hash HashArray(Span<const T> array) {
        usize seed = array.Length();
        for (const T& value : array) {
            seed ^= HashObject(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return AsHash(seed);
    }

    template <class T> struct Hasher {
        Hash operator()(const T& value) const { return value.GetHashCode(); }
    };

    template <Integer I> struct Hasher<I> {
        Hash operator()(I i) const { return HashInt((usize)i); }
    };

    template <> struct Hasher<void*> {
        Hash operator()(void* ptr) const { return HashInt(reinterpret_cast<usize>(ptr)); }
    };
    template <class T> struct Hasher<T*> : Hasher<void*> {};

    template <class T> Hash HashObject(const T& obj) {
        return Hasher<T> {} (obj);
    }

    struct DefaultHasher {
        Hash operator()(const auto& x) const { return HashObject(x); }
    };
}

namespace Quasi {
    template <class T, class Super>
    Hashing::Hash IContinuousCollection<T, Super>::GetHashCode() const {
        return Hashing::HashArray(AsSpan());
    }

    template <class T> Hashing::Hash Span<T>::GetHashCode() const {
        return Hashing::HashArray(*this);
    }

    template <class T> Hashing::Hash Ref<T>::GetHashCode() const {
        return Hashing::HashObject(ValueImpl());
    }

    template <class T> Hashing::Hash OptRef<T>::GetHashCode() const {
        return HasValueImpl() ? Hashing::HashObject(ValueImpl()) : Hashing::EmptyHash();
    }

    template <class T> Hashing::Hash Option<T>::GetHashCode() const {
        return HasValueImpl() ? Hashing::HashObject(UnwrapImpl()) : Hashing::EmptyHash();
    }

    inline Hashing::Hash OptionUsize::GetHashCode() const {
        return Hashing::HashInt(value);
    }
}