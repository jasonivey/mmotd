// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information_decls.h"

#include <cstdlib>
#include <string>
#include <string_view>

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

    // enum value: InformationId::ID_LOCATION_INFO_TIMEZONE
    InformationId GetId() const noexcept { return information_id_; }
    // string: "InformationId::ID_LOCATION_INFO_TIMEZONE"
    std::string GetIdStr() const noexcept { return id_str_; }
    // string: "ID_LOCATION_INFO_TIMEZONE"
    std::string GetPlainIdStr() const noexcept { return id_str_no_namespace_; }
    // string: "timezone"
    std::string GetName() const noexcept { return name_; }
    // string: "{}" in most all cases -- some floats are formatted as "{:.1f}"
    std::string GetFormat() const noexcept { return format_str_; }
    // string: "America/Denver" -- value after looked up via API call and formatted
    std::string GetValue() const noexcept { return value_; }

    void SetName(const std::string &new_name) { name_ = new_name; }

    void SetValue(std::string new_value) { value_ = new_value; }

    template<typename... Args>
    void SetValueArgs(Args &&...args) {
        SetValueImpl(std::string_view(format_str_), fmt::format_arg_store<fmt::format_context, Args...>(args...));
    }

private:
    void SetValueImpl(fmt::string_view format, fmt::format_args args) { value_ = fmt::vformat(format, args); }

    CategoryId category_ = CategoryId::ID_INVALID;
    InformationId information_id_ = InformationId::ID_INVALID_INVALID_INFORMATION;
    std::string id_str_;
    std::string id_str_no_namespace_;
    std::string name_;
    std::string format_str_;
    std::string value_;
};

} // namespace mmotd::information
