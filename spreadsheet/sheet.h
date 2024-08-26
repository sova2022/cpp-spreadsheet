#pragma once

#include "cell.h"
#include "common.h"

class Sheet : public SheetInterface {
public:
    Sheet();
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    const Cell* GetCellPtr(Position pos) const;
    Cell* GetCellPtr(Position pos);
    void ClearCell(Position pos) override;
    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    Size size_;

    void EnsurePositionIsValid(Position pos) const;
    void ResizeIfNeeded(Position pos);
    Size CalculateSize() const;
};

