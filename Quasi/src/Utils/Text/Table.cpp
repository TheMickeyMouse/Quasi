#include "Table.h"

namespace Quasi::Text {
    Table::Table(u32 cols) : alignments(Vec<Alignment>::WithSize(cols)), cols(cols) {
        alignments.Fill(Alignment::LEFT);
    }

    void Table::SetAlign(u32 col, Alignment align) {
        alignments[col] = align;
    }

    void Table::ReserveRows(u32 rows) {
        cells.Reserve(cols * rows);
    }

    void Table::AddCell(String cell) {
        cells.Push(std::move(cell));
    }

    void Table::Write(StringWriter sw) {
        // we *have* to do this pass before we start fmtting.
        Vec<u32> maxLengthOfCols = Vec<u32>::WithSize(cols);
        maxLengthOfCols.Fill(0);
        for (usize i = 0; i < cells.Length(); ++i) {
            const usize x = i % cols;
            maxLengthOfCols[x] = std::max(maxLengthOfCols[x], (u32)cells[i].Length());
        }

        bool isHeader = true;
        for (usize i = 0, col = 0; i < cells.Length(); ++i, ++col) {
            if (col == cols) {
                sw.Write("|\n");
                if (isHeader) {
                    for (u32 j = 0; j < col; ++j) {
                        sw.Write(j == 0 ? "| " :  " | ");
                        sw.WriteRepeat('-', maxLengthOfCols[j]);
                    }
                    sw.Write(" |\n");
                    isHeader = false;
                }
                col = 0;
            }
            sw << "| " << WithFormatOptions { Utf8Str(cells[i]), {
                .targetLength = maxLengthOfCols[col],
                .alignment = alignments[col]
            } } << ' ';
        }
        sw.Write('|');
    }
}
