#include "StringList.h"

#include "Type.h"
#include "Text.h"

namespace Quasi {
    Str StringListIter::CurrentImpl() const {
        const byte size = *ptr;
        return Str::Slice(ptr + 1, size);
    }

    void StringListIter::AdvanceImpl() {
        ptr += (*ptr) + 2;
    }

    bool StringListIter::CanNextImpl() const {
        return *ptr != '\0';
    }

    StringList StringList::FromListOf(IList<Str> strings) {
        StringList list;
        for (const auto str : strings) {
            list.Push(str);
        }
        return list;
    }

    StringList StringList::Only(Str first) {
        StringList list;
        list.stringlist += (char)first.Length();
        list.stringlist += first;
        list.stringlist += '\0';
        return list;
    }

    StringList StringList::FromListed(String slist) {
        return { std::move(slist), 1 };
    }

    void StringList::Push(Str str) {
        stringlist += (char)str.Length();
        stringlist += str;
        stringlist += '\0';
    }

    void StringList::Connect(StringListView list) {
        for (const auto str : list) Push(str);
    }

    StringListView StringList::AsView() const {
        return { *this };
    }

    StringListView::StringListView(const StringList& sl) : sv(sl.stringlist) {}

    String StringListView::Join(Str c) const {
        // return Text::ArrStr(*this, c);
    }

    Str StringListView::First() const {
        return sv.Substr(1, sv.First());
    }

    StringListView StringListView::Rest() const {
        return { sv.Substr(sv[0] + 2) };
    }
} // Q