// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "lib/include/platform/load_average.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

using namespace std;
using fmt::format;

constexpr static const char *LOAD_AVERAGE_FILENAME = "/proc/loadavg";

namespace {

optional<double> FromString(string str) {
    char *str_end = nullptr;
    auto value = strtod(data(str), &str_end);
    if (data(str) == str_end || errno == ERANGE) {
        LOG_ERROR("error converting string '{}' into a valid double", str);
        value = double{0.0};
    }
    return value;
}

optional<double> ParseLoadAverage(const string &line) {
    auto parts = vector<string>{};
    boost::split(parts, line, boost::is_any_of(" "), boost::token_compress_on);
    if (parts.size() < 2) {
        LOG_ERROR("unable to parse '{}' into valid load averages", line);
        return nullopt;
    }
    return FromString(boost::trim_copy(parts.front()));
}

optional<double> GetSystemLoadAverage() {
    auto load_average_file = ifstream{};
    load_average_file.exceptions(std::ifstream::goodbit);
    load_average_file.open(LOAD_AVERAGE_FILENAME, ios_base::in);

    if (!load_average_file.is_open() || load_average_file.fail() || load_average_file.bad()) {
        LOG_ERROR("unable to open {} for reading, {}",
                  LOAD_AVERAGE_FILENAME,
                  mmotd::error::ios_flags::to_string(load_average_file));
        return nullopt;
    }
    auto load_average_str = string{};
    getline(load_average_file, load_average_str);
    return ParseLoadAverage(load_average_str);
}

} // namespace

namespace mmotd::platform {

std::optional<double> GetLoadAverageDetails() {
    auto load_average_holder = GetSystemLoadAverage();
    if (load_average_holder.has_value() && !std::isnan(load_average_holder.value())) {
        return load_average_holder;
    }

    return nullopt;
}

} // namespace mmotd::platform
#endif
