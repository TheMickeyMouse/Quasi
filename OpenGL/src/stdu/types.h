#pragma once

namespace stdu {
    struct empty {
        empty(auto ...) {}
    }; // used with [[no_unique_address]]

    template <class T, class M> M decltype_member(M T::*) { return M {}; }

#define STDU_DECLTYPE_MEMBER(T, M) decltype(stdu::decltype_member(&T::M))
}
