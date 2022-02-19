// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
    static const auto informations = mmotd::information::Informations{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)                                                                  \
    Information(BOOST_PP_CAT(CategoryId::ID_, cat),                                                                    \
                             BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor))),                          \
                                                       BOOST_PP_STRINGIZE(BOOST_PP_CAT(InformationId::ID_, BOOST_PP_CAT(cat, BOOST_PP_CAT(_, id_descriptor)))),        \
                                                           name,                                                       \
                                                           fmttr),
#define CATEGORY_INFO_DEF(name, description, value)
#include "common/include/information_defs.h"
    };
    return informations;
}

InformationId from_string(std::string id_str) {
    const auto &informations = GetInformations();
    auto i = find_if(begin(informations), end(informations), [id_str](const Information &information) {
        return information.GetPlainIdStr() == id_str;
    });
    return i != end(informations) ? i->GetId() : InformationId::ID_INVALID_INVALID_INFORMATION;
}

std::string to_string(InformationId id) {
    const auto &informations = GetInformations();
    auto i = find_if(begin(informations), end(informations), [id](const Information &information) {
        return information.GetId() == id;
    });
    return i != end(informations) ? i->GetPlainIdStr() : "ID_INVALID_INVALID_INFORMATION"s;
}

const CategoryIds &GetCategoryIds() {
    static const auto category_ids = CategoryIds{
#define INFO_DEF(cat, id_descriptor, name, fmttr, id)
#define CATEGORY_INFO_DEF(name, description, value)                                                                    \
    std::make_pair(static_cast<CategoryId>(MakeCategoryId(value)), BOOST_PP_STRINGIZE(BOOST_PP_CAT(ID_, name))),
#include "common/include/information_defs.h"
    };
    return category_ids;
}

} // namespace mmotd::information
