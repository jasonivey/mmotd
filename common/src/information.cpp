// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
                         std::string format_str_,
                         InformationType information_) :
    category(category_),
    information_id(information_id_),
    repr(repr_),
    name(name_),
    format_str(format_str_),
    information(information_) {
}

string Information::to_string() const {
    // variant<boost::blank, bool, double, int32_t, std::string, size_t, uint64_t>;
    if (auto ptr = get_if<bool>(&information); ptr != nullptr) {
        return format("{}", *ptr);
    } else if (auto ptr = get_if<double>(&information); ptr != nullptr) {
        return ::to_string(*ptr);
    } else if (auto ptr = get_if<int32_t>(&information); ptr != nullptr) {
        return ::to_string(*ptr);
    } else if (auto ptr = get_if<string>(&information); ptr != nullptr) {
        return *ptr;
    } else if (auto ptr = get_if<size_t>(&information); ptr != nullptr) {
        return ::to_string(*ptr);
    } else if (auto ptr = get_if<uint64_t>(&information); ptr != nullptr) {
        return ::to_string(*ptr);
    } else {
        return string{};
    }
}

} // namespace mmotd::information
