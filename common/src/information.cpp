// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information.h"

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

namespace mmotd::information {

Information::Information(CategoryId category_,
                         InformationId information_id_,
                         std::string repr_,
                         std::string name_,
                         std::string format_str_) :
    category(category_),
    information_id(information_id_),
    repr(repr_),
    name(name_),
    format_str(format_str_),
    information(string{}) {
    mmotd::algorithms::unused(category);
}

//string Information::to_string() const {
//    return information;
//}

} // namespace mmotd::information
