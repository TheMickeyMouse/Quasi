#pragma once
#include "Type.h"

namespace Quasi {
    struct StringListIter {
        const char* ptr;
        [[nodiscard]] Str Value() const;
        void Advance();

        auto operator<=>(const StringListIter&) const = default;
        StringListIter& operator++() { Advance(); return *this; }
        [[nodiscard]] Str operator*() const { return Value(); }
    };

    struct StringList;

    struct StringListView {
        Str sv;
        StringListView() = default;
        StringListView(const StringList& sl);

        using Iter = StringListIter;

        [[nodiscard]] String Join(Str c) const;

        [[nodiscard]] Str First() const;
        [[nodiscard]] StringListView Rest() const;

        [[nodiscard]] bool Empty() const { return sv.empty(); }

        [[nodiscard]] Iter begin() const { return { sv.data() }; }
        [[nodiscard]] Iter end() const { return { sv.data() + sv.size() }; }
    };

    // represents a vector of strings as: [len1][string1...][len2][string2...]...
    struct StringList {
        String stringlist;

    private:
        StringList(String slist, int /* to seperate constructors */) : stringlist(std::move(slist)) {}
    public:
        StringList() = default;
        StringList(IList<Str> strings);
        StringList(Str str);
        StringList(const char* str) : StringList(Str(str)) {}

        static StringList FromListed(String slist);

        void Push(Str str);
        void Connect(StringListView list);

        [[nodiscard]] StringListView AsView() const;

        [[nodiscard]] Str First() const { return AsView().First(); }

        [[nodiscard]] String Join(Str c) const { return AsView().Join(c); }

        [[nodiscard]] bool Empty() const { return stringlist.empty(); }
        void Clear() { stringlist.clear(); }

        using Iter = StringListIter;

        [[nodiscard]] Iter begin() const { return { stringlist.data() }; }
        [[nodiscard]] Iter end() const { return { stringlist.data() + stringlist.size() }; }

        auto operator<=>(const StringList&) const = default;
    };

    inline StringList operator&(Str first, Str second) {
        return { first, second };
    }

    inline StringList operator&(StringList first, Str second) {
        first.Push(second);
        return std::move(first);
    }

    inline StringList operator&(StringList first, StringListView second) {
        first.Connect(second);
        return std::move(first);
    }
} // Q
