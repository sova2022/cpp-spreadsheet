#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell() override;

    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    bool HasDependentCells() const;
    void InvalidateCache();
    void FindCircularDependency(const std::vector<Position>& ref_cells, std::unordered_set<Cell*>& visited_cells);
    void FindCircularDependency(const std::vector<Position>& ref_cells);
    void UpdateDependencies();

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> dependent_cells_;
};