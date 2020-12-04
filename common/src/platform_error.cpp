// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/platform_error.h"

#include <cstdio>
#include <fmt/format.h>
#include <plog/Log.h>
#include <vector>

using namespace std;
using fmt::format;

namespace mmotd::platform::error {

string to_string(int error_value) {
    auto retval = int{0};
    auto buf_size = size_t{128};
    auto error_buf = vector<char>{};
    do {
        buf_size *= 2;
        error_buf.resize(buf_size, 0);
        retval = strerror_r(error_value, error_buf.data(), error_buf.size());
    } while (retval == ERANGE);
    if (retval == EINVAL) {
        PLOG_ERROR << format("error {} is not a valid error number", error_value);
        return format("error {} is not a valid error number", error_value);
    } else {
        assert(retval == 0); // strerror_r should be success(0) since all other errors are covered
        auto error_str = string{error_buf.begin(), error_buf.end()};
        PLOG_INFO << format("error {}: {}", error_value, error_str);
        return error_str;
    }
}

} // namespace mmotd::platform::error
