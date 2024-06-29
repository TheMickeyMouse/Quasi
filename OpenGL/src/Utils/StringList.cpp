#include "StringList.h"

#include "Type.h"
#include "Text.h"

namespace Quasi {
    Str StringListIter::Value() const {
        const byte size = *ptr;
        return Str { ptr + 1, size };
    }

    void StringListIter::Advance() {
        ptr += (*ptr) + 2;
    }

    StringList::StringList(IList<Str> strings) {
        for (const auto str : strings) {
            Push(str);
        }
    }

    StringList::StringList(Str str) {
        stringlist += (char)str.size();
        stringlist += str;
        stringlist += '\0';
    }

    StringList StringList::FromListed(String slist) {
        return { std::move(slist), 1 };
    }

    void StringList::Push(Str str) {
        stringlist += (char)str.size();
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
        return Text::ArrStr(*this, c);
    }

    Str StringListView::First() const {
        return *begin();
    }

    StringListView StringListView::Rest() const {
        return { sv.substr(sv[0] + 2) };
    }
} // Q