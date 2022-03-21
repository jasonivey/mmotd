// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information.h"
#include "common/include/information_decls.h"
#include "common/include/information_definitions.h"

#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std::string_literals;
using namespace std;

namespace mmotd::information {

const Informations &GetInformations() {
    // clang-format off
    static const auto informations = mmotd::information::Informations{
    /* Informations: unordered_map<InformationId, vector<Information>>                                                  */
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                                      \
    /* Key: InformationId                                                                                               */ \
    {BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))),                                  \
    /* Value: vector<Information> which at this point will always be 1 Information                                      */ \
     {Information(BOOST_PP_CAT(CategoryId::ID_, cat),                                                                      \
                  BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))),                     \
                  BOOST_PP_STRINGIZE(BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor)))), \
                  name,                                                                                                    \
                  fmttr)},},
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
    };
    // clang-format on
    return informations;
}

const vector<Information> &GetInformationList() {
    // clang-format off
    static const auto informations = vector<Information>{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                                      \
    Information(BOOST_PP_CAT(CategoryId::ID_, cat),                                                                        \
                BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))),                       \
                BOOST_PP_STRINGIZE(BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor)))),   \
                name,                                                                                                      \
                fmttr),
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
    };
    // clang-format on
    return informations;
}

InformationId from_information_id_string(std::string id_str) {
    // clang-format off
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                                      \
    if (id_str == BOOST_PP_STRINGIZE(BOOST_PP_CAT(ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))))) {              \
        return BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor)));                        \
    }
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
    // clang-format on
    return InformationId::ID_INVALID_INVALID_INFORMATION;
}

std::string to_string(InformationId id) {
    switch (id) {
        // clang-format off
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                                      \
        case BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))):                          \
            return BOOST_PP_STRINGIZE(BOOST_PP_CAT(ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))));               \
            break;
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
        // clang-format on
        case InformationId::ID_INVALID_INVALID_INFORMATION:
        default:
            return "ID_INVALID_INVALID_INFORMATION"s;
    }
}

const CategoryIds &GetCategoryIds() {
    // clang-format off
    static const auto category_ids = CategoryIds{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)
#define CATEGORY_INFO_DEF(name, description, value)                                                                        \
    std::make_pair(static_cast<CategoryId>(MakeCategoryId(value)), BOOST_PP_STRINGIZE(BOOST_PP_CAT(ID_, name))),
#include "common/include/information_defs.h"
    };
    // clang-format on
    return category_ids;
}

} // namespace mmotd::information
