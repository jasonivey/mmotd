// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include "common/results/include/template_column_items.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

namespace mmotd::results {

using ColumnIndex = int;

class ColumnIndexes {
public:
    using container = std::vector<ColumnIndex>;
    using value_type = ColumnIndex;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

    ColumnIndexes() = default;
    ColumnIndexes(ColumnIndexes const &other) = default;
    ColumnIndexes &operator=(ColumnIndexes const &other) = default;
    ColumnIndexes(ColumnIndexes &&other) = default;
    ColumnIndexes &operator=(ColumnIndexes &&other) = default;
    ~ColumnIndexes() = default;

    explicit ColumnIndexes(std::initializer_list<ColumnIndex> il) : indexes_(il) { sort(); }

    std::size_t size() const noexcept { return indexes_.size(); }
    bool empty() const noexcept { return indexes_.empty(); }
    bool contains(ColumnIndex i) const noexcept;
    std::string to_string() const;

    reference operator[](std::size_t i) noexcept { return indexes_[i]; }
    const_reference operator[](std::size_t i) const noexcept { return indexes_[i]; }

    reference front() { return indexes_.front(); }
    const_reference front() const { return indexes_.front(); }

    iterator begin() noexcept { return indexes_.begin(); }
    const_iterator begin() const noexcept { return indexes_.begin(); }
    const_iterator cbegin() const noexcept { return indexes_.cbegin(); }

    iterator end() noexcept { return indexes_.end(); }
    const_iterator end() const noexcept { return indexes_.end(); }
    const_iterator cend() const noexcept { return indexes_.cend(); }

    void insert(ColumnIndex value) noexcept;

    static void sort(container &indexes) noexcept;

private:
    void sort() noexcept;

    container indexes_;
};

} // namespace mmotd::results
