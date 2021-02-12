// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/chrono_io.h"

#include <chrono>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

namespace mmotd::information {

enum class CategoryId : size_t;
enum class InformationId : size_t;

class Information {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(Information);

    Information(CategoryId category,
                InformationId information_id,
                std::string id_str,
                std::string name,
                std::string format_str);

    InformationId GetId() const { return information_id_; }
    std::string GetIdStr() const { return id_str_; }
    std::string GetName() const { return name_; }
    std::string GetFormat() const { return format_str_; }
    std::string GetValue() const { return value_; }

    void SetName(const std::string &new_name) { name_ = new_name; }

    template<typename... Args>
    void SetValueFormat(const std::string &fmt_str, Args &&...args) {
        SetValueImpl(fmt_str, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void SetValueArgs(Args &&...args) {
        SetValueImpl(format_str_, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    void SetValueImpl(std::string_view fmt_str, Args &&...args) {
        value_ = fmt::format(fmt_str, std::forward<Args>(args)...);
    }

    CategoryId category_;
    InformationId information_id_;
    std::string id_str_;
    std::string name_;
    std::string format_str_;
    std::string value_;
};

} // namespace mmotd::information
