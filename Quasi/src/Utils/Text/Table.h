#pragma once
#include "Formatting.h"
#include "Utils/String.h"

namespace Quasi::Text {
    struct Table {
        using Alignment = TextFormatOptions::Alignment;
        Vec<String> cells;
        Vec<Alignment> alignments;
        u32 cols;

        Table(u32 cols);

        void SetAlign(u32 col, Alignment align);
        void SetLeft(u32 col)   { SetAlign(col, Alignment::LEFT);   }
        void SetCenter(u32 col) { SetAlign(col, Alignment::CENTER); }
        void SetRight(u32 col)  { SetAlign(col, Alignment::RIGHT);  }

        void ReserveRows(u32 rows);
        void AddCell(String cell);
        template <class... Ts>
        void AddCellFmt(Str fmt, const Ts& ...args) {
            AddCell(Text::Format(fmt, args...));
        }

        void Write(StringWriter sw);
    };
}
