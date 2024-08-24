#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) try
            : ast_(ParseFormulaAST(std::move(expression))) { 
        }
        catch (const std::exception& exc) {
            throw FormulaException(exc.what());
        }


        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            }
            catch (const FormulaError& err) {
                return err;
            }
        }

        std::string GetExpression() const override {
            try {
                std::ostringstream out;
                ast_.PrintFormula(out);
                return out.str();
            }
            catch (const std::exception& exc) {
                throw FormulaException(exc.what());
            }
        }

        std::vector<Position> GetReferencedCells() const override {
            const auto& pos_list = ast_.GetCells();
            std::set<Position> s(pos_list.begin(), pos_list.end());
            return { s.begin(), s.end() };
        }
    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}