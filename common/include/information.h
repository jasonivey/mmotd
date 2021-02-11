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

    Information(CategoryId category_,
                InformationId information_id_,
                std::string repr_,
                std::string name_,
                std::string format_str_);

    InformationId GetId() const { return information_id; }
    std::string GetValue() const { return information; }
    std::string GetName() const { return name; }
    std::string GetFormat() const { return format_str; }

    void SetName(const std::string &new_name) { name = new_name; }

    template<typename T>
    void SetValue(T value, const std::string &fmt_str) {
        SetValueImpl(fmt_str, value);
    }

    template<typename T>
    void SetValue(T value) {
        SetValueImpl(format_str, value);
    }

#if 0
    template<class T, typename = std::enable_if_t<mmotd::type_traits::is_time_point<T>::value>
    void SetValue(T timepoint) {
        SetValueImpl(format_str, value);
    }

    template<class T, typename = std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value>>
    void SetValue(T value) {
        SetValueImpl(format_str, value);
    }

    template<>
    void SetValue<>(std::string value) {
        SetValueImpl(format_str, value);
    }
#endif

    //std::string to_string() const;
    std::string to_repr() const { return repr; }

private:
    template<typename T>
    void SetValueImpl(std::string_view fmt_str, T value) {
        information = fmt::format(fmt_str, value);
    }

    CategoryId category;
    InformationId information_id;
    std::string repr;
    std::string name;
    std::string format_str;
    std::string information;
};

} // namespace mmotd::information
