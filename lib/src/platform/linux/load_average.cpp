// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#if defined(__linux__)
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "lib/include/platform/load_average.h"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>

#include <math.h>
#include <sys/sysinfo.h>

using namespace std;
using fmt::format;

constexpr static const char *LOAD_AVERAGE_FILENAME = "/proc/loadavg";

namespace {

optional<int32_t> GetCpuCount() {
    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count == -1) {
        LOG_ERROR("sysconf returned -1 calling get processor count _SC_NPROCESSORS_ONLN");
        return nullopt;
    } else {
        LOG_INFO("sysconf for _SC_NPROCESSORS_ONLN returned {} processors", cpu_count);
        return make_optional(cpu_count);
    }
}

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

LoadAverageDetails GetLoadAverageDetails() {
    auto cpu_count_holder = GetCpuCount();
    int32_t cpu_count = cpu_count_holder.has_value() ? cpu_count_holder.value() : int32_t{0};

    auto load_average_holder = GetSystemLoadAverage();
    double load_average = 0.0;
    if (load_average_holder.has_value() && !isnan(load_average_holder.value())) {
        load_average = load_average_holder.value();
    }

    return make_tuple(cpu_count, load_average);
}

} // namespace mmotd::platform
#endif
