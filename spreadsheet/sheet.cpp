#include "sheet.h"

#include <algorithm>
#include <iostream>

Sheet::Sheet() : size_{ 0, 0 } {}

void Sheet::EnsurePositionIsValid(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
}

void Sheet::ResizeIfNeeded(Position pos) {
    EnsurePositionIsValid(pos);
    if (pos.row >= size_.rows || pos.col >= size_.cols) {
        size_.rows = std::max(size_.rows, pos.row + 1);
        size_.cols = std::max(size_.cols, pos.col + 1);
        cells_.resize(size_.rows);
        for (auto& row : cells_) {
            row.resize(size_.cols);
        }
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    ResizeIfNeeded(pos);
    if (!cells_[pos.row][pos.col]) {
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    else {
        if (cells_[pos.row][pos.col]->GetText() == text) {
            return; 
        }
    }
    cells_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);
}

CellInterface* Sheet::GetCell(Position pos) {    
    return const_cast<CellInterface*>(static_cast<const Sheet&>(*this).GetCell(pos));
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    EnsurePositionIsValid(pos);
    if (pos.row < size_.rows && pos.col < size_.cols) {
        return cells_[pos.row][pos.col].get();
    }
    return nullptr;
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

void Sheet::ClearCell(Position pos) {
    EnsurePositionIsValid(pos);
    if (pos.row < size_.rows && pos.col < size_.cols) {
        auto& cell = cells_[pos.row][pos.col];
        if (cell && cell->HasDependentCells()) {
            cell->Clear();
        }
        else {
            cell.reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return CalculateSize();
}

Size Sheet::CalculateSize() const {
    Size printable_size{0, 0};
    for (int row = 0; row < size_.rows; ++row) {
        for (int col = 0; col < size_.cols; ++col) {
            if (cells_[row][col] && !cells_[row][col]->GetText().empty()) {
                printable_size.rows = std::max(printable_size.rows, row + 1);
                printable_size.cols = std::max(printable_size.cols, col + 1);
            }
        }
    }
    return printable_size;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (const CellInterface* cell = GetCell({ row, col })) {
                auto cell_value = cell->GetValue();
                auto output_value = [&output](const auto& value) {
                    output << value;
                    };

                std::visit(output_value, cell_value);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (cells_[row][col]) {
                output << cells_[row][col]->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}


