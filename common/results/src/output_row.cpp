// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/information.h"
#include "common/include/informations.h"
#include "common/results/include/output_position_index.h"
#include "common/results/include/output_row.h"
#include "common/results/include/template_string.h"

#include <limits>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using mmotd::information::Informations;
using mmotd::results::data::ENTIRE_LINE;
using mmotd::results::data::ENTIRE_LINE_REPR;
using mmotd::results::data::TemplateColumnItem;

using namespace std;

namespace mmotd::results {

Row::Row(TemplateColumnItem item) : item_(item) {
}

int Row::GetRowNumber() const noexcept {
    return item_.row_index;
}

PositionIndex Row::GetColumnPosition() const noexcept {
    return position_index_;
}

int Row::GetColumnNumber() const noexcept {
    return position_index_.GetColumnNumber();
}

string Row::GetColumnNumberStr() const noexcept {
    auto column_number = GetColumnNumber();
    return column_number == ENTIRE_LINE ? string{ENTIRE_LINE_REPR} : std::to_string(column_number);
}

size_t Row::GetColumnCount() const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    MMOTD_PRECONDITIONS(!empty(names_) && !empty(values_), "output names and values must not be empty");
    return empty(names_.front()) || empty(values_.front()) ? size_t{1} : size_t{2};
}

size_t Row::GetHeight() const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return size(names_);
}

void Row::SetPositionIndex(PositionIndex position_index) {
    MMOTD_PRECONDITIONS(position_index.IsPositionValid(), "position index must be one of the Position values");
    position_index_ = position_index;
}

bool Row::BalanceNameValueSize() {
    auto count = max(size(names_), size(values_));
    if (count == 0) {
        return false;
    }
    if (size(names_) > size(values_)) {
        count = size(names_) - size(values_);
        for (auto i = size_t{0}; i != count; ++i) {
            AddValue(string{});
        }
    } else {
        count = size(values_) - size(names_);
        for (auto i = size_t{0}; i != count; ++i) {
            AddName(string{});
        }
    }
    MMOTD_CHECKS(size(names_) == size(values_), "output names and values are not equal sizes");
    return true;
}

size_t Row::UpdateRowNumber(int row_number) {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    PLOG_VERBOSE << format(FMT_STRING("updating row number: {} to: {} for name: '{}', value: '{}'"),
                           item_.row_index,
                           row_number,
                           GetName(0),
                           GetValue(0));
    item_.row_index = row_number;
    return size(names_);
}

void Row::SetNameValue(const Informations &informations) {
    for (const auto &sub_name : item_.name) {
        auto template_string = TemplateString{sub_name};
        auto modified_name =
            template_string.TransformTemplateName(informations, item_, static_cast<size_t>(item_.repeatable_index));
        PLOG_VERBOSE << format(FMT_STRING("adding name to row {}: '{}'"), GetRowNumber(), modified_name);
        AddName(modified_name);
    }
    for (const auto &sub_value : item_.value) {
        auto template_string = TemplateString{sub_value};
        auto modified_value =
            template_string.TransformTemplateValue(informations, item_, static_cast<size_t>(item_.repeatable_index));
        PLOG_VERBOSE << format(FMT_STRING("adding value to row {}: '{}'"), GetRowNumber(), modified_value);
        AddValue(modified_value);
    }
}

bool Row::HasName(size_t index) const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    MMOTD_PRECONDITIONS(index < size(names_), "index must be less than the size of names");
    return !empty(names_[index]);
}

bool Row::HasValue(size_t index) const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    MMOTD_PRECONDITIONS(index < size(values_), "index must be less than the size of values");
    return !empty(values_[index]);
}

string Row::GetName(size_t index) const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return index < size(names_) ? names_[index] : string{};
}

string Row::GetValue(size_t index) const {
    MMOTD_PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return index < size(values_) ? values_[index] : string{};
    return values_[index];
}

RowId Row::GenerateRowId() {
    static auto row_id_generator = boost::uuids::random_generator{};
    return row_id_generator();
}

} // namespace mmotd::results
