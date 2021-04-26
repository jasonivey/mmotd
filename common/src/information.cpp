// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information.h"
#include "common/include/logging.h"

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std;

namespace mmotd::information {

Information::Information(CategoryId category,
                         InformationId information_id,
                         string id_str,
                         string name,
                         string format_str) :
    category_(category),
    information_id_(information_id),
    id_str_(id_str),
    name_(name),
    format_str_(format_str),
    value_() {
    mmotd::algorithms::unused(category_);
}

} // namespace mmotd::information
