// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace mmotd::results {

class Row;

class Table {
public:
    Table(std::string table_style_str, std::size_t column_count);
    ~Table();

    void WriteRow(const Row &row);
    std::string to_string() const;

private:
    class TableImpl;

    std::unique_ptr<TableImpl> table_;
};

} // namespace mmotd::results
