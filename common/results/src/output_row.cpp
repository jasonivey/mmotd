// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/information.h"
#include "common/include/informations.h"
#include "common/include/logging.h"
#include "common/include/string_utils.h"
#include "common/results/include/output_position_index.h"
#include "common/results/include/output_row.h"
#include "common/results/include/template_string.h"

#include <limits>
#include <string>
#include <utility>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/format.h>

using fmt::format;
using mmotd::information::Informations;
using mmotd::results::data::ENTIRE_LINE;
using mmotd::results::data::ENTIRE_LINE_REPR;
using mmotd::results::data::TemplateColumnItem;

using namespace std;

namespace mmotd::results {

Row::Row(TemplateColumnItem item) : item_(std::move(item)) {}

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
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    PRECONDITIONS(!empty(names_) && !empty(values_), "output names and values must not be empty");
    return empty(names_.front()) || empty(values_.front()) ? size_t{1} : size_t{2};
}

size_t Row::GetHeight() const {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return size(names_);
}

void Row::SetPositionIndex(PositionIndex position_index) {
    PRECONDITIONS(position_index.IsPositionValid(), "position index must be one of the Position values");
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
    CHECKS(size(names_) == size(values_), "output names and values are not equal sizes");
    return true;
}

size_t Row::UpdateRowNumber(int row_number) {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    LOG_VERBOSE("updating row number: {} to: {} for name: '{}', value: '{}'",
                item_.row_index,
                row_number,
                GetName(0),
                GetValue(0));
    item_.row_index = row_number;
    return size(names_);
}

void Row::SetNameValue(const Informations &informations) {
    for (auto i = size_t{0}; i != size(item_.name); ++i) {
        const auto &sub_name = item_.name[i];
        auto sub_color = item_.GetNameColor(i);
        auto sub_row_name = format(FMT_STRING("{}{}"), GetRowNumber(), string(size_t{1}, 'a' + i));
        if (empty(sub_name)) {
            LOG_VERBOSE("skipping row {} since it's name is empty", sub_row_name);
            continue;
        }
        auto information_index = static_cast<size_t>(item_.repeatable_index);
        auto name_xfr_ids = TemplateString::ReplaceInformationIds(sub_name, informations, information_index);
        if (empty(name_xfr_ids)) {
            LOG_VERBOSE("skipping row {} since the 'name' converted from information ids is empty", sub_row_name);
            continue;
        }
        auto modified_name = TemplateString::ReplaceEmbeddedColorCodes(name_xfr_ids, sub_color);
        LOG_VERBOSE("adding to row {}, name: '{}'", sub_row_name, modified_name);
        AddName(modified_name);
    }
    for (auto i = size_t{0}; i != size(item_.value); ++i) {
        const auto &sub_value = item_.value[i];
        auto sub_color = item_.GetValueColor(i);
        auto sub_row_name = format(FMT_STRING("{}{}"), GetRowNumber(), string(size_t{1}, 'a' + i));
        if (empty(sub_value)) {
            LOG_VERBOSE("skipping row {} since it's value is empty", sub_row_name);
            continue;
        }
        auto information_index = static_cast<size_t>(item_.repeatable_index);
        auto value_xfr_ids = TemplateString::ReplaceInformationIds(sub_value, informations, information_index);
        if (empty(value_xfr_ids)) {
            LOG_VERBOSE("skipping row {} since the 'value' converted from information ids is empty", sub_row_name);
            continue;
        }
        auto modified_value = TemplateString::ReplaceEmbeddedColorCodes(value_xfr_ids, sub_color);
        LOG_VERBOSE("adding to row {}, value: '{}'", sub_row_name, modified_value);
        AddValue(modified_value);
    }
}

bool Row::HasName(size_t index) const {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    PRECONDITIONS(index < size(names_), "index must be less than the size of names");
    return !empty(names_[index]);
}

bool Row::HasValue(size_t index) const {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    PRECONDITIONS(index < size(values_), "index must be less than the size of values");
    return !empty(values_[index]);
}

string Row::GetName(size_t index) const {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return index < size(names_) ? names_[index] : string{};
}

string Row::GetValue(size_t index) const {
    PRECONDITIONS(size(names_) == size(values_), "output names and values must be equal sizes");
    return index < size(values_) ? values_[index] : string{};
    return values_[index];
}

RowId Row::GenerateRowId() {
    static auto row_id_generator = boost::uuids::random_generator{};
    return row_id_generator();
}

} // namespace mmotd::results
