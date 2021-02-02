// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstdlib>
#include <string>
#include <variant>

#include <boost/blank.hpp>

namespace mmotd::information {

enum class CategoryId : size_t;
enum class InformationId : size_t;

using InformationType = std::variant<boost::blank, bool, double, int32_t, std::string, size_t, uint64_t>;

class Information {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(Information);

    Information(CategoryId category_,
                InformationId information_id_,
                std::string repr_,
                std::string name_,
                std::string format_str_,
                InformationType information_);

    CategoryId category;
    InformationId information_id;
    std::string repr;
    std::string name;
    std::string format_str;
    InformationType information;

    std::string to_string() const;
    std::string to_repr() const { return repr; }
};

} // namespace mmotd::information
