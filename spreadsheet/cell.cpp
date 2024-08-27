#include "cell.h"
#include "sheet.h"

#include <iostream>
#include <optional>

class Cell::Impl {
public:
    virtual ~Impl() = default;
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }

    virtual std::optional<FormulaInterface::Value> GetCache() const {
        return std::nullopt;
    }

    virtual void ResetCache() {}
};

class Cell::EmptyImpl : public Impl {
public:
    CellInterface::Value GetValue() const override {
        return "";
    }

    std::string GetText() const override {
        return "";
    }
};

class Cell::TextImpl : public Impl {
public:
    explicit TextImpl(std::string text) : text_(std::move(text)) {}

    CellInterface::Value GetValue() const override {
        if (!text_.empty() && text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }

    std::string GetText() const override {
        return text_;
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(std::string expression, const SheetInterface& sheet)
        : formula_(ParseFormula(std::move(expression)))
        , sheet_(sheet) {
    }
    
    CellInterface::Value GetValue() const override {
        auto formula_value = formula_->Evaluate(sheet_);
        if (!cache_.has_value()) {
            cache_ = formula_value;
        }
        if (std::holds_alternative<double>(formula_value)) {
            return std::get<double>(formula_value);
        }
        else {
            return std::get<FormulaError>(formula_value);
        }
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }

    std::optional<FormulaInterface::Value> GetCache() const {
        return cache_;
    }

    void ResetCache() {
        cache_.reset();
    }

private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface& sheet_;
    mutable std::optional<FormulaInterface::Value> cache_;
};

// Cell
Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet) {
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> temp_impl;
    if (text.empty()) {
        temp_impl = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        temp_impl = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        FindCircularDependency(temp_impl->GetReferencedCells());
    }
    else {
        temp_impl = std::make_unique<TextImpl>(std::move(text));
    }
    impl_ = std::move(temp_impl);
    UpdateDependencies();
    InvalidateCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::HasDependentCells() const {
    return !dependent_cells_.empty();
}

void Cell::FindCircularDependency(const std::vector<Position>& ref_cells, std::unordered_set<Cell*>& visited_cells) {
    for (const auto& pos : ref_cells) {
        Cell* referenced_cell = sheet_.GetCellPtr(pos);
        if (referenced_cell == this) {
            throw CircularDependencyException("Circular dependency detected");
        }
        if (referenced_cell && !visited_cells.count(referenced_cell)) {
            const auto& another_ref_cells = referenced_cell->GetReferencedCells();
            if (!another_ref_cells.empty()) {
                FindCircularDependency(another_ref_cells, visited_cells);
            }
            visited_cells.insert(referenced_cell);
        }
    }
}

void Cell::FindCircularDependency(const std::vector<Position>& ref_cells) {
    std::unordered_set<Cell*> visited_cells;
    FindCircularDependency(ref_cells, visited_cells);
}

void Cell::UpdateDependencies() {
    for (const auto& pos : impl_->GetReferencedCells()) {
        if (!sheet_.GetCell(pos)) {
            sheet_.SetCell(pos, "");
        }
        Cell* new_referenced_cell = sheet_.GetCellPtr(pos);
        new_referenced_cell->dependent_cells_.insert(this);
    }
}

void Cell::InvalidateCache() {
    impl_->ResetCache();
    for (const auto& dependent_cell : dependent_cells_) {
        if (dependent_cell->impl_->GetCache()) {
            dependent_cell->InvalidateCache();
        }
    }
}

