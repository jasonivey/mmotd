// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/information.h"
#include "common/include/logging.h"

#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

using fmt::format;
using namespace std;

static constexpr auto INFORMATION_ID_PREFIX = string_view{"InformationId::"};

namespace mmotd::information {

Information::Information(CategoryId category,
                         InformationId information_id,
                         string id_str,
                         string name,
                         string format_str) :
    category_(category),
    information_id_(information_id),
    id_str_(std::move(id_str)),
    id_str_no_namespace_(
        id_str_.substr(id_str_.starts_with(INFORMATION_ID_PREFIX) ? INFORMATION_ID_PREFIX.size() : 0ull)),
    name_(std::move(name)),
    format_str_(std::move(format_str)),
    value_() {
    mmotd::algorithms::unused(category_);
}

} // namespace mmotd::information
