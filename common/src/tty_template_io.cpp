// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/informations.h"
#include "common/include/tty_template.h"
#include "common/include/tty_template_data.h"
#include "common/include/tty_template_io.h"
#include "common/include/tty_template_string.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iterator>
#include <optional>
#include <regex>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <fmt/color.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

using mmotd::algorithms::join;
using mmotd::information::Information;
using mmotd::information::InformationId;
using mmotd::information::Informations;
using mmotd::tty_template::OutputTemplate;
using mmotd::tty_template::data::TemplateColumnItem;
using mmotd::tty_template::data::TemplateColumnItems;
using mmotd::tty_template::tty_string::TemplateString;

namespace {

constexpr const size_t COLUMN_SEPERATOR_WIDTH = 3;

struct NameValue {
    string name;
    string value;
    size_t name_width = 0ull;
    size_t value_width = 0ull;
};

//using NameValue = tuple<string, size_t, string>;
using NameValues = vector<NameValue>;

struct DisplayableNameValue {
    DisplayableNameValue() = default;
    DisplayableNameValue(int column, int row) : column(column), row(row) {}
    int column = 0;
    int row = -1;
    NameValues name_values;
};

using DisplayableNameValues = vector<DisplayableNameValue>;
using MultipleDisplayableNameValues = vector<DisplayableNameValues>;
using IndexedDisplayableNameValues = unordered_map<int, DisplayableNameValues>;
using GroupedTemplateColumnItems = unordered_map<int, TemplateColumnItems>;

struct ColumnData {
    vector<string> name;
    vector<string> value;
    size_t name_width = size_t{0};
    size_t value_width = size_t{0};
    int row_index = 0;
    int column_index = 0;
    size_t sub_row_count = 0;
};

using ColumnDatas = vector<ColumnData>;
using ColumnDatasMap = tuple<int, ColumnDatas>;
using RowColumnsIndexes = vector<ColumnDatasMap>;

using ColumnStrs = vector<string>;
using ColumnIdentifier = tuple<int, int, ColumnStrs>;
using ColumnIdentifiers = vector<ColumnIdentifier>;

enum class PositionIndex { FirstLast, First, Middle, Last };

string to_string(PositionIndex index) {
    // enum class PositionIndex { First, Middle, Last, FirstLast };
    if (index == PositionIndex::FirstLast) {
        return "FirstLast";
    } else if (index == PositionIndex::First) {
        return "First";
    } else if (index == PositionIndex::Last) {
        return "Last";
    } else {
        return "Middle";
    }
}

template<typename It>
auto GetPositionIndex(It begin, It /*end*/, It iter) {
    // enum class PositionIndex { First, Middle, Last, FirstLast };
    // fix_jasoni: hack!!!
    if (iter == begin) {
        // PositionIndex::FirstLast is only index 0
        return PositionIndex::FirstLast;
    } else if (iter == begin + 1) {
        // PositionIndex::First is only index 1
        return PositionIndex::First;
    } else if (iter == begin + 2) {
        // PositionIndex::Last is only index 2
        return PositionIndex::Last;
    } else {
        return PositionIndex::Middle;
    }
    // if (iter == begin && iter + 1 == end) {
    //     return PositionIndex::FirstLast;
    // } else if (iter == begin) {
    //     return PositionIndex::First;
    // } else if (iter == end || iter + 1 == end) {
    //     return PositionIndex::Last;
    // } else {
    //     return PositionIndex::Middle;
    // }
}

template<typename It>
auto GetPositionIndex(It begin, It end, size_t index) {
    return GetPositionIndex(begin, end, begin + index);
}

template<typename Container>
auto GetPositionIndex(const Container &container, const typename Container::value_type &value) {
    auto i = find(std::begin(container), std::end(container), value);
    return GetPositionIndex(std::begin(container), std::end(container), i);
}

TemplateColumnItems AddOrRemoveColumnItems(TemplateColumnItems items, const Informations &informations) {
    sort(begin(items), end(items), [](const auto &a, const auto &b) { return a.row_index < b.row_index; });
    for (auto i = std::begin(items); i != std::end(items);) {
        auto item = *i;
        auto item_value = std::empty(item.value) ? string{} : item.value.front();
        if (std::empty(item_value)) {
            PLOG_VERBOSE << format("item at index={} does not have a value", distance(begin(items), i));
            ++i;
            continue;
        } else if (!item.is_optional && !item.is_repeatable) {
            PLOG_VERBOSE << format("item is not optional and not repeatable, value=\"{}\"", item_value);
            ++i;
            continue;
        }
        // This is all sorts of wrong since I'm guessing that each value has an "%ID_%" of some within it
        auto id = TemplateString{item_value}.GetFirstInformationId(informations);
        if (id == InformationId::ID_INVALID_INVALID_INFORMATION) {
            PLOG_VERBOSE << format("unable to find information id for value=\"{}\"", item_value);
            ++i;
            continue;
        }
        auto count = informations.count(id);
        if (count == 0 && item.is_optional) {
            PLOG_VERBOSE << format("item is optional and count=0 for value=\"{}\"", item_value);
            i = items.erase(i);
        } else if (count > 1 && item.is_repeatable) {
            // #error this structure needs to be ordered by row_index before doing modifications
            // #error then row_index needs to be only incremented by +1 not an independent variable (i.e. row_index)
            // #error this is becoming way too complex and you haven't pushed in weeks!!!
            PLOG_VERBOSE << format("item is repeatable and count={} for value=\"{}\"", count, item_value);
            PLOG_VERBOSE << format("adding {} identical item{} for value=\"{}\"",
                                   count - 1,
                                   (count - 1 > 1 ? string{"s"} : string{}),
                                   item_value);
            auto column_index = item.column;
            i = items.insert(i, count - 1, *i);
            // auto row_index = item.row_index;
            for (auto j = i + 1; j != std::end(items); ++j) {
                auto &item_ref = *j;
                item_value = std::empty(item_ref.value) ? string{} : item_ref.value.front();
                if (item_ref.column != 0 && column_index != item_ref.column) {
                    PLOG_VERBOSE << format("not updating row={}, column={}, only updating columns 0 and {}",
                                           item_ref.row_index,
                                           item_ref.column,
                                           column_index);
                    continue;
                }
                if (auto repeatable_index = static_cast<size_t>(distance(i, j)); repeatable_index < count) {
                    item_ref.repeatable_index = repeatable_index;
                    PLOG_VERBOSE << format("row_index={}, repeatable_index={} for item=\"{}\"",
                                           item_ref.row_index,
                                           item_ref.repeatable_index,
                                           item_value);
                }
                PLOG_VERBOSE << format("row_index={} is now row_index={} for item=\"{}\"",
                                       item_ref.row_index,
                                       item_ref.row_index + count - 1,
                                       item_value);
                item_ref.row_index += count - 1;
            }
            i += count;
        } else {
            ++i;
        }
    }

    for (const auto &item : items) {
        auto item_value = std::empty(item.value) ? string{} : item.value.front();
        PLOG_VERBOSE << format("item=[{}][{}]: \"{}\"", item.column, item.row_index, item_value);
    }
    return items;
}

auto GetTemplateColumnItemColumn(GroupedTemplateColumnItems &grouped_items, int column) {
    auto i = grouped_items.find(column);
    if (i != end(grouped_items)) {
        return i;
    } else {
        auto result = grouped_items.insert(make_pair(column, TemplateColumnItems{}));
        return result.second ? result.first : end(grouped_items);
    }
}

auto GroupColoumnItemsByColumns(TemplateColumnItems items, vector<int> columns) {
    // for (auto column : columns) {
    //     fmt::print("column: {}\n", column);
    // }
    auto grouped_items = GroupedTemplateColumnItems{};
    for (auto column : columns) {
        auto i = GetTemplateColumnItemColumn(grouped_items, column);
        if (i == end(grouped_items)) {
            PLOG_ERROR << format("unable to find/create an entry in the template column index map for column {}",
                                 column);
            continue;
        }
        auto &grouped_items_for_column = (*i).second;
        copy_if(begin(items), end(items), back_inserter(grouped_items_for_column), [column](const auto &item) {
            auto name = empty(item.name) ? string{} : item.name.front();
            auto value = empty(item.value) ? string{} : item.value.front();
            // fmt::print("evaluating item column: {}, name: {}, value: {}\n", item.column, name, value);
            return item.column == column;
        });
    }
    // for (const auto &[column_id, template_column_items] : grouped_items) {
    //     for (auto item : template_column_items) {
    //         auto name = empty(item.name) ? string{} : item.name.front();
    //         auto value = empty(item.value) ? string{} : item.value.front();
    //         fmt::print("colected column: {}, name: {}, value: {}\n", column_id, name, value);
    //     }
    // }
    return grouped_items;
}

size_t GetNameColumnMaxWidth(const DisplayableNameValues &displayable_name_values) {
    auto width = size_t{0};
    for (const auto &displayable_name_value : displayable_name_values) {
        auto i = max_element(cbegin(displayable_name_value.name_values),
                             cend(displayable_name_value.name_values),
                             [](const auto &a, const auto &b) { return a.name_width < b.name_width; });
        if (i != cend(displayable_name_value.name_values)) {
            width = std::max((*i).name_width, width);
        }
    }
    return width + 1;
}

size_t GetValueColumnMaxWidth(const DisplayableNameValues &displayable_name_values) {
    auto width = size_t{0};
    for (const auto &displayable_name_value : displayable_name_values) {
        for (const auto &name_value : displayable_name_value.name_values) {
            if (!empty(name_value.name)) {
                width = max(width, name_value.value_width);
            }
        }
    }
    return width;
}

void SetNameColumnMaxWidth(DisplayableNameValues &displayable_name_values, size_t max_width) {
    for (auto &displayable_name_value : displayable_name_values) {
        for (auto &name_value : displayable_name_value.name_values) {
            name_value.name_width = max_width;
        }
    }
}

void SetValueColumnMaxWidth(DisplayableNameValues &displayable_name_values, size_t max_width) {
    for (auto &displayable_name_value : displayable_name_values) {
        for (auto &name_value : displayable_name_value.name_values) {
            name_value.value_width = max_width;
        }
    }
}

// enum class PositionIndex { FirstLast, First, Middle, Last };
NameValue AddCustomizations(string name, string value, PositionIndex index, const TemplateColumnItem &item) {
    PLOG_VERBOSE << format(
        "adding customizations, name={}, value={}, index={}, prepend newlines={}, indent size={}, append newlines={}",
        name,
        value,
        to_string(index),
        item.prepend_newlines,
        item.indent_size,
        item.append_newlines);
    if (item.prepend_newlines > 0 && (index == PositionIndex::First || index == PositionIndex::FirstLast)) {
        PLOG_VERBOSE << format("prepending {} newline to the beginning of {}={}",
                               item.prepend_newlines,
                               std::empty(name) ? "value" : "name",
                               std::empty(name) ? value : name);
        if (!std::empty(name)) {
            name.insert(begin(name), item.prepend_newlines, '\n');
        } else {
            value.insert(begin(value), item.prepend_newlines, '\n');
        }
    }
    if (item.indent_size > 0 && (index == PositionIndex::First || index == PositionIndex::FirstLast)) {
        PLOG_VERBOSE << format("indenting {} spaces to the beginning of {}={}",
                               item.indent_size,
                               std::empty(name) ? "value" : "name",
                               std::empty(name) ? value : name);
        if (!std::empty(name)) {
            name.insert(begin(name), item.indent_size, ' ');
        } else {
            value.insert(begin(value), item.indent_size, ' ');
        }
    }
    if (item.append_newlines > 0 && (index == PositionIndex::FirstLast || index == PositionIndex::Last)) {
        PLOG_VERBOSE << format("adding {} newlines to the end of {}={}",
                               item.append_newlines,
                               std::empty(value) ? "name" : "value",
                               std::empty(value) ? name : value);
        if (!std::empty(value)) {
            value.insert(end(value), item.append_newlines, '\n');
        } else {
            name.insert(end(name), item.append_newlines, '\n');
        }
    }
    PLOG_VERBOSE << format(
        "customizations finished, name={}, value={}, index={}, prepend newlines={}, indent size={}, append newlines={}",
        name,
        value,
        to_string(index),
        item.prepend_newlines,
        item.indent_size,
        item.append_newlines);
    return NameValue{name, value, std::size(name), std::size(value)};
}

auto AddTransformedNameValues(const TemplateColumnItem &item,
                              const std::vector<int> &column_indexes,
                              const vector<string> &names,
                              const vector<string> &values) {
    if (size(names) == size(values)) {
        // (most all cases) either one or multiple name/values
        auto displayable_name_value = DisplayableNameValue{item.column, item.row_index};
        for (auto i = 0ull; i != size(names); ++i) {
            auto position_index = GetPositionIndex(begin(column_indexes), end(column_indexes), item.column);
            displayable_name_value.name_values.emplace_back(
                AddCustomizations(names[i], values[i], position_index, item));
        }
        return displayable_name_value;
    } else if (size(names) == 1 && size(values) > 1) {
        // it is a multiple line "value" -- the first line has a name
        auto displayable_name_value = DisplayableNameValue{item.column, item.row_index};
        for (auto i = 0ull; i != size(values); ++i) {
            auto position_index = GetPositionIndex(begin(column_indexes), end(column_indexes), item.column);
            if (i == 0 && !empty(names[i])) {
                displayable_name_value.name_values.emplace_back(
                    AddCustomizations(names[i], values[i], position_index, item));
            } else {
                displayable_name_value.name_values.emplace_back(
                    AddCustomizations(string{}, values[i], position_index, item));
            }
        }
        return displayable_name_value;
    } else if (empty(names) && !empty(values)) {
        // it is an entire line "value" -- no associated name
        auto displayable_name_value = DisplayableNameValue{item.column, item.row_index};
        for (auto i = 0ull; i != size(values); ++i) {
            auto position_index = GetPositionIndex(begin(column_indexes), end(column_indexes), item.column);
            displayable_name_value.name_values.emplace_back(
                AddCustomizations(string{}, values[i], position_index, item));
        }
        return displayable_name_value;
    } else {
        // should never be the case where !empty(names) && empty(values)
        PLOG_VERBOSE << format("found an unknown case where names size={} and values size={}",
                               size(names),
                               size(values));
        return DisplayableNameValue{item.column, item.row_index};
    }
}

DisplayableNameValues CreateDisplayableNameValues(TemplateColumnItems column_items,
                                                  const std::vector<int> &column_indexes,
                                                  const Informations &informations,
                                                  size_t column_index,
                                                  size_t column_count) {
    auto displayable_name_values = DisplayableNameValues{};
    for (const auto &column_item : column_items) {
        auto names = vector<string>{};
        for (const auto &sub_name : column_item.name) {
            auto template_string = TemplateString{sub_name};
            auto modified_name =
                template_string.TransformTemplateName(informations, column_item, column_item.repeatable_index);
            names.push_back(modified_name);
        }
        auto values = vector<string>{};
        for (const auto &sub_value : column_item.value) {
            auto template_string = TemplateString{sub_value};
            auto modified_value =
                template_string.TransformTemplateValue(informations, column_item, column_item.repeatable_index);
            values.push_back(modified_value);
        }
        displayable_name_values.emplace_back(AddTransformedNameValues(column_item, column_indexes, names, values));
    }
    // remove any completely empty informational items... something messed up (template, information retrieval or parsing)?
    auto i = remove_if(begin(displayable_name_values),
                       end(displayable_name_values),
                       [](const auto &displayable_name_value) { return empty(displayable_name_value.name_values); });
    displayable_name_values.erase(i, end(displayable_name_values));

    // these functions are called on a column worth of data
    auto name_column_width = GetNameColumnMaxWidth(displayable_name_values);
    SetNameColumnMaxWidth(displayable_name_values, name_column_width);

    if (column_index != 0 && column_count > 1) {
        auto value_column_width = GetValueColumnMaxWidth(displayable_name_values);
        SetValueColumnMaxWidth(displayable_name_values, value_column_width);
    }

    return displayable_name_values;
}

DisplayableNameValues FlattenDisplayableItems(MultipleDisplayableNameValues &name_values_grouped_by_column) {
    auto displayable_items_flattened = DisplayableNameValues{};
    for (auto displayable_name_values : name_values_grouped_by_column) {
        copy(begin(displayable_name_values), end(displayable_name_values), back_inserter(displayable_items_flattened));
    }
    sort(begin(displayable_items_flattened), end(displayable_items_flattened), [](const auto &a, const auto &b) {
        return a.row > b.row;
    });
    return displayable_items_flattened;
}

string GetColumnNameValueFormattedLayout(NameValue name_value, PositionIndex position_index) {
    if (!std::empty(name_value.name) && !std::empty(name_value.value)) {
        if (position_index == PositionIndex::First || position_index == PositionIndex::Middle) {
            PLOG_VERBOSE << format("position: {}, name width: {}, value width: {}, name: {}, value: {}",
                                   to_string(position_index),
                                   name_value.name_width,
                                   name_value.value_width,
                                   name_value.name,
                                   name_value.value);
            auto padding = string(name_value.value_width - size(name_value.value), ' ');
            return format("{:{}} {}{}", name_value.name, name_value.name_width, name_value.value, padding);
        } else {
            PLOG_VERBOSE << format("position: {}, name width: {}, name: {}, value: {}",
                                   to_string(position_index),
                                   name_value.name_width,
                                   name_value.name,
                                   name_value.value);
            return format("{:{}} {}", name_value.name, name_value.name_width, name_value.value);
        }
    } else if (std::empty(name_value.name) && !std::empty(name_value.value)) {
        PLOG_VERBOSE << format("position: {}, value: {}", to_string(position_index), name_value.value);
        return format("{}", name_value.value);
    } else if (!std::empty(name_value.name) && std::empty(name_value.value)) {
        PLOG_VERBOSE << format("position: {}, name: {}", to_string(position_index), name_value.name);
        return format("{}", name_value.name, name_value.name_width);
    } else {
        PLOG_ERROR << format("name value is empty, position: {}", to_string(position_index));
        return string{};
    }
}

string GetColumnNameValueLayout(NameValue name_value, PositionIndex position_index) {
    auto result = GetColumnNameValueFormattedLayout(name_value, position_index);
    if (position_index == PositionIndex::First || position_index == PositionIndex::Middle) {
        result = boost::replace_all_copy(result, "\n", "");
    } else if (position_index == PositionIndex::Last) {
        result = boost::trim_left_copy_if(result, boost::is_any_of("\n"));
    }
    PLOG_VERBOSE << format("position: {}, layout: \"{}\"", to_string(position_index), empty(result) ? "empty" : result);
    return result;
}

RowColumnsIndexes GetRowColumnsIndexes(TemplateColumnItems items) {
    //vector<tuple<int, vector<int>>>
    auto row_columns_indexes = RowColumnsIndexes{};
    for (const auto &item : items) {
        auto column_data = ColumnData{};
        column_data.row_index = item.row_index;
        column_data.column_index = item.column;
        column_data.sub_row_count = max(size(item.name), size(item.value));

        auto i = find_if(begin(row_columns_indexes), end(row_columns_indexes), [&item](const auto &row_column_index) {
            const auto &[row, column_data_ignore] = row_column_index;
            return row == item.row_index;
        });
        if (i == end(row_columns_indexes)) {
            row_columns_indexes.push_back(make_tuple(item.row_index, ColumnDatas{column_data}));
        } else {
            auto &[row, column_datas] = *i;
            column_datas.push_back(column_data);
        }
    }

    sort(begin(row_columns_indexes), end(row_columns_indexes), [](const auto &a, const auto &b) {
        auto &[row_a, column_data_a] = a;
        auto &[row_b, column_data_b] = b;
        return row_a < row_b;
    });

    for_each(begin(row_columns_indexes), end(row_columns_indexes), [](auto &row_column_index) {
        auto &[row_ignore, column_datas] = row_column_index;
        sort(begin(column_datas), end(column_datas), [](const auto &a, const auto &b) {
            return a.column_index < b.column_index;
        });
    });

    for (const auto &row_columns_index : row_columns_indexes) {
        const auto &[row, column_datas] = row_columns_index;
        for (const auto &column_data : column_datas) {
            PLOG_VERBOSE << format(
                "row_columns_indexes, row: {}, column: {}, sub row: {}, name width: {}, value width: {}, name: \"{}\", value: \"{}\"",
                column_data.row_index,
                column_data.column_index,
                column_data.sub_row_count,
                column_data.name_width,
                column_data.value_width,
                boost::join(column_data.name, ", "),
                boost::join(column_data.value, ", "));
        }
    }

    return row_columns_indexes;
}

optional<NameValues>
GetColumnData(const DisplayableNameValues &displayable_name_values, size_t row_index, size_t column_index) {
    for (auto i = begin(displayable_name_values); i != end(displayable_name_values); ++i) {
        const auto &displayable_name_value = *i;
        if (displayable_name_value.row == static_cast<int>(row_index) &&
            displayable_name_value.column == static_cast<int>(column_index)) {
            return make_optional(displayable_name_value.name_values);
        }
    }
    return nullopt;
}

struct TColumnData {
    int column = 0;
    vector<string> values;
};

struct RowData {
    int row = 0;
    vector<TColumnData> columns;
};

using Rows = vector<RowData>;

vector<int> GetRowIndexes(const Rows &rows) {
    auto row_indexes = vector<int>{};
    transform(cbegin(rows), cend(rows), back_inserter(row_indexes), [](const auto &row_data) { return row_data.row; });
    sort(begin(row_indexes), end(row_indexes), std::less());
    return row_indexes;
}

vector<int> GetColumnIndexes(const RowData &row_data) {
    auto column_indexes = vector<int>{};
    for (const auto &column_data : row_data.columns) {
        auto i = lower_bound(begin(column_indexes), end(column_indexes), column_data.column);
        if (i == end(column_indexes)) {
            column_indexes.push_back(column_data.column);
        } else if (*i != column_data.column) {
            column_indexes.insert(i, column_data.column);
        }
        PLOG_VERBOSE << format("column indexes ({}): [{}]",
                               size(column_indexes),
                               join(column_indexes, ", ", [](auto i) { return std::to_string(i); }));
    }
    PLOG_VERBOSE << format("column indexes ({}): [{}]", size(column_indexes), join(column_indexes, ", ", [](auto i) {
                               return std::to_string(i);
                           }));
    // sort(begin(column_indexes), end(column_indexes), std::less());
    return column_indexes;
}

size_t GetMaxColumnCount(const Rows &rows) {
    size_t column_count = size_t{0};
    for (const auto &row_data : rows) {
        for (const auto &column_data : row_data.columns) {
            column_count = max(column_count, static_cast<size_t>(column_data.column));
        }
    }
    // the stored column identifier is zero based -- convert it to a count
    return column_count + 1;
}

void AddColumn(Rows &rows, int row_number, int column_number, const string &column_str) {
    PLOG_VERBOSE << format("AddColumn input, row: {}, column: {}, str: \"{}\"", row_number, column_number, column_str);
    auto i = find_if(begin(rows), end(rows), [row_number](const auto &row) { return row.row == row_number; });
    if (i != end(rows)) {
        auto &row_data = *i;
        auto j = find_if(begin(row_data.columns), end(row_data.columns), [column_number](const auto &column) {
            return column.column == column_number;
        });
        if (j != end(row_data.columns)) {
            auto &column_data = *j;
            column_data.values.push_back(column_str);
        } else {
            row_data.columns.push_back(TColumnData{column_number, vector<string>{column_str}});
        }
    } else {
        rows.push_back(
            RowData{row_number, vector<TColumnData>{TColumnData{column_number, vector<string>{column_str}}}});
    }
}

optional<RowData> GetRowData(const Rows &rows, int row_index) {
    auto i = find_if(cbegin(rows), cend(rows), [row_index](const auto &row_data) { return row_data.row == row_index; });
    if (i != cend(rows)) {
        return make_optional(*i);
    } else {
        return nullopt;
    }
}

optional<TColumnData> GetColumnData(const vector<TColumnData> &columns, int column_index) {
    auto i = find_if(cbegin(columns), cend(columns), [column_index](const auto &column_data) {
        return column_data.column == column_index;
    });
    if (i != cend(columns)) {
        return make_optional(*i);
    } else {
        return nullopt;
    }
}

template<typename It>
vector<string> GetColumnRange(It begin, It end, size_t level) {
    const auto count = static_cast<size_t>(distance(begin, end));
    auto result = vector<string>(count);
    for (auto i = 0ull; i < count; ++i) {
        const auto &column_values = *(begin + i);
        if (level < size(column_values)) {
            result[i] = column_values[level];
        }
    }
    return result;
}

size_t GetMaxValuesInColumns(const vector<vector<string>> &columns, size_t columns_per_row, size_t index) {
    auto max_values = size_t{0};
    for (auto i = index; i < index + columns_per_row && i < size(columns); ++i) {
        max_values = max(max_values, size(columns[i]));
    }
    return max_values;
}

bool IsRootColumn(const vector<vector<string>> &columns, size_t columns_per_row, size_t index) {
    auto root_column = true;
    for (auto i = index; i < index + columns_per_row && i < size(columns); ++i) {
        root_column = i == index ? !empty(columns[i]) : empty(columns[i]);
        if (!root_column) {
            return root_column;
        }
    }
    return root_column;
}

string CombineRootColumn(const vector<vector<string>> &columns, size_t index) {
    return index < size(columns) ? boost::join(columns[index], "") : string{};
}

string RemoveColorCodes(string input) {
    // all color codes start with 0x1b, "[", other stuff in the middle, ending with "m"
    PLOG_VERBOSE << format("removing color codes from: {}", input);
    const auto color_code_regex = regex(R"(\x1b\[[^m]+m)", std::regex::ECMAScript);
    // write the results to an output iterator
    auto output = regex_replace(input, color_code_regex, "");
    PLOG_VERBOSE << format("removed color codes from: {}", output);

    return output;
}

string GetColumnIdentifier(size_t row_number, size_t column_number, size_t column_index, size_t value_index) {
    return format("row: {}, column: {}, col index: {}, index: {}",
                  row_number,
                  column_number,
                  column_index,
                  value_index);
}

string GetColumnPadding(const vector<vector<string>> &columns,
                        size_t columns_per_row,
                        size_t base_index,
                        size_t column_index,
                        size_t value_index) {
    // couple of variables for debug output
    const auto row_number = (base_index + columns_per_row) / columns_per_row;
    const auto column_number = column_index % columns_per_row;
    // no need for this calculation on column{0} and the base index better be <ge> than column - 1 == what we'll use for format width
    if (column_index == 0 || base_index >= column_index - 1) {
        PLOG_VERBOSE << format("{}, skipping padding, column_index[{}] == 0 || base_index[{}] < column_index[{}] - 1",
                               GetColumnIdentifier(row_number, column_number, column_index, value_index),
                               column_index,
                               base_index,
                               column_index);
        return string{};
    }
    // column 0 is unique and takes the entire row, row 1 is already padded for the full column
    if (column_number < 2) {
        PLOG_VERBOSE << format("{}, skipping padding since padding is only necessary on columns > 2: {}",
                               GetColumnIdentifier(row_number, column_number, column_index, value_index),
                               column_number);
        return string{};
    }
    // if we are not adding the second value in a multi-string value then there is nothing to pad
    if (value_index < 1) {
        PLOG_VERBOSE << format("{}, skipping padding because value_index: {} < 1",
                               GetColumnIdentifier(row_number, column_number, column_index, value_index),
                               value_index);
        return string{};
    }
    const auto previous_column_index = column_index - 1;

    // if the previous column was not empty empty then padding is not needed
    if (value_index < size(columns[previous_column_index]) && !empty(columns[previous_column_index][value_index])) {
        PLOG_VERBOSE << format(
            "{}, skipping padding because previous column wasn't empty, value_index: {} >= size(columns[previous_column_index: {}]: {}): {}",
            GetColumnIdentifier(row_number, column_number, column_index, value_index),
            value_index,
            previous_column_index,
            boost::join(columns[previous_column_index], ", "),
            size(columns[previous_column_index]));
        return string{};
    }

    const auto previous_index = value_index - 1;
    if (previous_index >= size(columns[previous_column_index])) {
        PLOG_VERBOSE << format(
            "{}, skipping padding previous_index: {} >= size(columns[previous_column_index: {}]: {}): {}",
            GetColumnIdentifier(row_number, column_number, column_index, value_index),
            previous_index,
            previous_column_index,
            boost::join(columns[previous_column_index], ", "),
            size(columns[previous_column_index]));
        return string{};
    }

    auto previous_value = columns[previous_column_index][previous_index];
    PLOG_VERBOSE << format(
        "row: {}, column: {}, col index: {}, index: {}, using string for size of padding: {}, size: {}",
        row_number,
        column_number,
        column_index,
        value_index,
        previous_value,
        size(previous_value));

    previous_value = RemoveColorCodes(previous_value);
    auto spaces_length = size(previous_value) + COLUMN_SEPERATOR_WIDTH;

    PLOG_VERBOSE << format(
        "row: {}, column: {}, col index: {}, index: {}, using updated string for size of padding: {}, size: {}",
        row_number,
        column_number,
        column_index,
        value_index,
        previous_value,
        spaces_length);
    return string(spaces_length, ' ');
}

string GetColumnString(const vector<vector<string>> &columns,
                       vector<size_t> &column_sizes,
                       size_t columns_per_row,
                       size_t base_index,
                       size_t column_index,
                       size_t value_index) {
    if (value_index >= size(columns[column_index])) {
        return string{};
    }
    const auto row_number = (base_index + columns_per_row) / columns_per_row;
    const auto column_number = column_index % columns_per_row;
    auto padding = GetColumnPadding(columns, columns_per_row, base_index, column_index, value_index);
    const auto &column_str = columns[column_index][value_index];
    // auto column_value = format("{}{}{}", padding, column_str, column_number > 0 ? string{"  "} : string{});
    auto column_value = format("{}{}", padding, column_str);
    if (column_number > 0 && column_number < columns_per_row - 1) {
        column_value += string(COLUMN_SEPERATOR_WIDTH, ' ');
    }
    column_sizes[column_number] = size(column_value);
    PLOG_VERBOSE << format("row: {}, column: {}, col index: {}, adding index: {}, value: \"{}\"",
                           row_number,
                           column_number,
                           column_index,
                           value_index,
                           column_value);
    return column_value;
}

string CombineRowColumns(const vector<vector<string>> &columns, size_t columns_per_row, size_t index) {
    auto row_str = string{};
    auto column_sizes = vector<size_t>(columns_per_row, 0ull);
    const auto max_values = GetMaxValuesInColumns(columns, columns_per_row, index);
    const auto row_number = (index + columns_per_row) / columns_per_row;
    PLOG_VERBOSE << format("max values in row: {} is: {}", row_number, max_values);
    for (auto value_index = 0ull; value_index < max_values; ++value_index) {
        for (auto column_index = index; column_index < index + columns_per_row && column_index < size(columns);
             ++column_index) {
            const auto column_number = column_index % columns_per_row;
            PLOG_VERBOSE << format("looking at value in row: {}, column: {}, col index: {}, index: {}",
                                   row_number,
                                   column_number,
                                   column_index,
                                   value_index);
            row_str += GetColumnString(columns, column_sizes, columns_per_row, index, column_index, value_index);
            PLOG_VERBOSE << format("row: {}, column: {}, col index: {}, adding index: {}, concat value: \"{}\"",
                                   row_number,
                                   column_number,
                                   column_index,
                                   value_index,
                                   row_str);
        }
    }
    PLOG_VERBOSE << format("row: {}, final value: \"{}\"", row_number, row_str);
    return row_str;
}

vector<string> LayoutColumnsInRows(const Rows &rows) {
    const auto max_columns = GetMaxColumnCount(rows);
    auto columns_strs = vector<vector<string>>{};
    for (auto row_index : GetRowIndexes(rows)) {
        auto row_holder = GetRowData(rows, row_index);
        if (!row_holder) {
            continue;
        }
        auto row_data = *row_holder;
        auto base_index = size(columns_strs);
        columns_strs.insert(end(columns_strs), max_columns, vector<string>{});
        for (auto column_index : GetColumnIndexes(row_data)) {
            auto column_holder = GetColumnData(row_data.columns, column_index);
            if (!column_holder) {
                continue;
            }
            auto column_data = *column_holder;
            auto column_strs_index = base_index + column_index;
            for (const auto &value : column_data.values) {
                columns_strs[column_strs_index].push_back(value);
            }
        }
    }
    for (auto columns_str : columns_strs) {
        for (auto column_str : columns_str) {
            // auto joined_column = format("\"{}\", boost::join(column_str, "\", \"");
            PLOG_VERBOSE << format("LayoutColumnsInRows middle, column: \"{}\"", column_str);
        }
    }
    auto rows_of_columns = vector<string>{};
    for (auto i = 0ull; i < size(columns_strs); i += max_columns) {
        if (IsRootColumn(columns_strs, max_columns, i)) {
            rows_of_columns.push_back(CombineRootColumn(columns_strs, i));
        } else {
            rows_of_columns.push_back(CombineRowColumns(columns_strs, max_columns, i));
        }
    }
    for (auto row_str : rows_of_columns) {
        PLOG_VERBOSE << format("LayoutColumnsInRows end, row: \"{}\"", row_str);
    }
    return rows_of_columns;
}

string LayoutRowsColumns(const Rows &rows) {
    // slice the [row7[column-a, column-b], row7[column-c]] layout into [row7[column-a, column-c], row7[<spacer>, column-b]]
    for (const auto &row : rows) {
        for (const auto &column : row.columns) {
            auto joined_column = format("\"{}\"", boost::join(column.values, "\", \""));
            PLOG_VERBOSE << format("LayoutRowsColumns start, row: {}, column: {}, values({}): {}",
                                   row.row,
                                   column.column,
                                   size(column.values),
                                   joined_column);
        }
    }
    auto rows_strs = LayoutColumnsInRows(rows);
    auto all_rows = boost::join(rows_strs, "");
    return all_rows;
}

//void PrintDisplayableNameValuesRow(DisplayableNameValues displayable_name_values,
void PrintDisplayableNameValues(DisplayableNameValues displayable_name_values,
                                const RowColumnsIndexes row_columns_indexes,
                                const std::vector<int> &column_indexes) {
    // using namespace std::placeholders;
    auto rows = Rows{};
    for (const auto &[row_index, column_datas] : row_columns_indexes) {
        for (const auto &column_data : column_datas) {
            const auto column_index = column_data.column_index;
            auto name_values_holder = GetColumnData(displayable_name_values, row_index, column_index);
            if (!name_values_holder) {
                PLOG_VERBOSE << format("unable to find row[{}], column[{}]", row_index, column_index);
                continue;
            }
            auto name_values = name_values_holder.value();
            auto position_index = GetPositionIndex(begin(column_indexes), end(column_indexes), column_index);
            for (const auto &name_value : name_values) {
                auto column_str = GetColumnNameValueLayout(name_value, position_index);
                PLOG_VERBOSE << format("row: [{}], column: [{}], position: {}, value: \"{}\"",
                                       row_index,
                                       column_index,
                                       to_string(position_index),
                                       column_str);
                AddColumn(rows, row_index, column_index, column_str);
            }
        }
    }
    for (const auto &row : rows) {
        for (const auto &column : row.columns) {
            PLOG_VERBOSE << format("AddColumn finished, row: {}, column: {}, values: \"{}\"",
                                   row.row,
                                   column.column,
                                   boost::join(column.values, ", "));
        }
    }

    auto rows_str = LayoutRowsColumns(rows);
    PLOG_VERBOSE << format("all rows: \"{}\"", rows_str);
    fmt::print(rows_str);
}

} // namespace

namespace mmotd::tty_template {

void PrintOutputTemplate(const OutputTemplate &output_template, const Informations &informations) {
    const auto column_indexes = output_template.GetColumns();
    auto column_items = AddOrRemoveColumnItems(output_template.GetColumnItems(), informations);

    sort(begin(column_items), end(column_items), [](const auto &a, const auto &b) { return a.column > b.column; });

    // for (const auto &item : column_items) {
    //     auto name = empty(item.name) ? string{} : item.name.front();
    //     auto value = empty(item.value) ? string{} : item.value.front();
    //     fmt::print("item column={}, row={}, name={}, value={}\n", item.column, item.row_index, name, value);
    // }

    auto items_grouped_by_column = GroupColoumnItemsByColumns(column_items, column_indexes);

    auto name_values_grouped_by_column = MultipleDisplayableNameValues{};
    for (const auto &[column_id, template_column_items] : items_grouped_by_column) {
        // for (auto item : template_column_items) {
        //     auto name = empty(item.name) ? string{} : item.name.front();
        //     auto value = empty(item.value) ? string{} : item.value.front();
        //     fmt::print("column: {}, name: {}, value: {}\n", column_id, name, value);
        // }
        auto name_values = CreateDisplayableNameValues(template_column_items,
                                                       column_indexes,
                                                       informations,
                                                       column_id,
                                                       size(items_grouped_by_column));
        name_values_grouped_by_column.push_back(name_values);
    }

    auto displayable_name_values = FlattenDisplayableItems(name_values_grouped_by_column);

    //auto name_values_grouped_by_row = GroupDisplayableItemsByRow(displayable_name_values);

    const auto row_columns_indexes = GetRowColumnsIndexes(column_items);

    PrintDisplayableNameValues(displayable_name_values, row_columns_indexes, column_indexes);
}

} // namespace mmotd::tty_template
