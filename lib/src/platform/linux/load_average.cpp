// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/iostream_error.h"
#include "lib/include/platform/load_average.h"

#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

#include <sys/sysinfo.h>

using namespace std;
using fmt::format;

constexpr static const char *LOAD_AVERAGE_FILENAME = "/proc/loadavg";

namespace {

optional<int32_t> GetCpuCount() {
    auto cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count == -1) {
        PLOG_ERROR << "sysconf returned -1 calling get processor count _SC_NPROCESSORS_ONLN";
        return nullopt;
    } else {
        PLOG_INFO << format("sysconf for _SC_NPROCESSORS_ONLN returned {} processors", cpu_count);
        return make_optional(cpu_count);
    }
}

optional<double> FromString(string str) {
    try {
        auto value = stod(str, nullptr);
        return make_optional(value);
    } catch (const std::invalid_argument &ex) {
        PLOG_ERROR << format("unable to convert {} to double: {}", str, ex.what());
    } catch (const std::out_of_range &ex) {
        PLOG_ERROR << format("unable to convert {} to double: {}", str, ex.what());
    } catch (const std::exception &ex) {
        PLOG_ERROR << format("unable to convert {} to double: {}.  RETHROWING", str, ex.what());
        throw;
    }
    return nullopt;
}

optional<double> ParseLoadAverage(const string &line) {
    auto parts = vector<string>{};
    boost::split(parts, line, boost::is_any_of(" "), boost::token_compress_on);
    if (parts.size() < 2) {
        PLOG_ERROR << format("unable to parse '{}' into valid load averages", line);
        return nullopt;
    }
    return FromString(boost::trim_copy(parts.front()));
}

optional<double> GetLoadAverage() {
    auto load_average_file = ifstream{};
    load_average_file.exceptions(std::ifstream::goodbit);
    load_average_file.open(LOAD_AVERAGE_FILENAME, ios_base::in);

    if (!load_average_file.is_open() || load_average_file.fail() || load_average_file.bad()) {
        PLOG_ERROR << format("unable to open {} for reading, {}",
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
    auto details = LoadAverageDetails{};

    auto cpu_count_wrapper = GetCpuCount();
    if (cpu_count_wrapper) {
        auto cpu_count = *cpu_count_wrapper;
        details.push_back(make_tuple("processor count", to_string(cpu_count)));
    }

    auto load_average_wrapper = GetLoadAverage();
    if (load_average_wrapper) {
        auto load_average = *load_average_wrapper;
        details.push_back(make_tuple("load average", format("{:.02f}", load_average)));
    }
    return details;
}

} // namespace mmotd::platform
