#pragma once
#include "String.h"
#include "Type.h"

namespace Quasi {
    struct StringListIter : IIterator<Str, StringListIter> {
        const char* ptr;
        StringListIter(const char* p) : ptr(p) {}

        Str  CurrentImpl() const;
        void AdvanceImpl();
        bool CanNextImpl() const;
    };

    struct StringList;

    struct StringListView {
        Str sv;

        StringListView() = default;
        StringListView(Str sv) : sv(sv) {}
        StringListView(const StringList& sl);

        using Iter = StringListIter;

        String Join(Str c) const;

        Str First() const;
        StringListView Rest() const;

        bool IsEmpty() const { return sv.IsEmpty(); }
    };

    // represents a vector of strings as: [len1][string1...][len2][string2...]...
    struct StringList {
        String stringlist;
    private:
        StringList(String slist, int /* to seperate constructors */) : stringlist(std::move(slist)) {}
    public:
        StringList() = default;

        static StringList FromListOf(IList<Str> strings);
        static StringList Only(Str first);

        static StringList FromListed(String slist);

        void Push(Str str);
        void Connect(StringListView list);

        StringListView AsView() const;

        Str First() const { return AsView().First(); }

        String Join(Str c) const { return AsView().Join(c); }

        bool Empty() const { return stringlist.IsEmpty(); }
        void Clear() { stringlist.Clear(); }

        StringListIter Iter() const { return { stringlist.Data() }; }
        bool operator==(const StringList&) const = default;
    };

    inline StringList operator&(Str first, Str second) {
        return { first, second };
    }

    inline StringList operator&(StringList first, Str second) {
        first.Push(second);
        return first;
    }

    inline StringList operator&(StringList first, StringListView second) {
        first.Connect(second);
        return first;
    }
} // Q
