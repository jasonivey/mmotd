// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/information.h"

#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

namespace mmotd::information {

// Make a category id: just the high 32 bits of a 64 bit unsigned integer
inline constexpr size_t MakeCategoryId(size_t id) {
    static_assert(sizeof(size_t) == sizeof(uint64_t));
    static_assert(std::is_unsigned<size_t>::value);
    return (id & 0x00000000FFFFFFFF) << 32;
}

// Take the 64 bit input value and ensure the low 32 bits are all that
//  are used for the information id part.  See MakeInformationId
inline constexpr size_t PrepValueForInformationId(size_t id) {
    return id & 0x00000000FFFFFFFF;
}

enum class CategoryId : size_t {
    ID_INVALID = MakeCategoryId(1ull),
#define INFO_DEF(cat, id_descriptor, name, fmttr, variant_type, id)
#define CATEGORY_INFO_DEF(cat_name, description, cat_value) BOOST_PP_CAT(ID_, cat_name) = MakeCategoryId(cat_value),
#include "common/include/information_defs.h"
};

// Take the 64 bit category id value (should already be 0xFFFFFFFF00000000
//  where only the upper 32 bits are used.  Prep the 64 bit id value by
//  clearing the upper 32-bits and finally or-ing these two numbers:
//  category-id            : 0000000C 00000000 = CategoryId::ID_SWAP_USAGE
//  information input val  : 00000000 0000001A = will be the value ID_SWAP_USAGE_PERCENT_USED
//  [logical-or] the values: -----------------
//  resulting InformationId: 0000000C 0000001A = InformationId::ID_SWAP_USAGE_PERCENT_USED
inline constexpr size_t MakeInformationId(CategoryId category, size_t id) {
    return static_cast<size_t>(category) | PrepValueForInformationId(id);
}

enum class InformationId : size_t {
    ID_INVALID_INVALID_INFORMATION = MakeInformationId(CategoryId::ID_INVALID, 1ull),
#define INFO_DEF(cat, id_descriptor, name, fmttr, variant_type, id)        \
    BOOST_PP_CAT(ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))) = \
        MakeInformationId(BOOST_PP_CAT(CategoryId::ID_, cat), id),
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
};

class Informations;
Informations CreateInformations();

using CategoryIds = std::vector<std::tuple<CategoryId, std::string>>;
CategoryIds CreateCategoryIds();

} // namespace mmotd::information
