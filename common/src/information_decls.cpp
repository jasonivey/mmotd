// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information_decls.h"
#include "common/include/information_definitions.h"
#include "common/include/logging.h"

#include <array>
#include <string>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std;

namespace mmotd::information {

Informations CreateInformations() {
    return mmotd::information::Informations{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                \
    Information{BOOST_PP_CAT(CategoryId::ID_, cat),                                                  \
                BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))), \
                BOOST_PP_STRINGIZE(BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor)))), \
                                                name,                                                \
                                                fmttr},
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
    };
}

CategoryIds CreateCategoryIds() {
    return CategoryIds{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)
#define CATEGORY_INFO_DEF(name, description, value) \
    make_tuple(static_cast<CategoryId>(MakeCategoryId(value)), BOOST_PP_STRINGIZE(BOOST_PP_CAT(ID_, name))),
#include "common/include/information_defs.h"
    };
}

} // namespace mmotd::information
