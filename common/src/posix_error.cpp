// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/posix_error.h"

#include <cstdio>
#include <string>
#include <system_error>
#include <vector>

#include <fmt/format.h>
#include <plog/Log.h>

#include <errno.h>

using namespace std;
using fmt::format;

namespace mmotd::error::posix_error {

string to_string() {
    if (errno == 0) {
        PLOG_DEBUG << "attempting to convert errno(0) to error string (i.e. returning nothing)";
        return string{};
    }
#if !defined(USE_STR_ERROR_CLIB)
    return make_error_code(static_cast<errc>(errno)).message();
#else
    auto error_value = errno;
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
        //PLOG_VERBOSE << format("error {}: {}", error_value, error_str);
        return format("{}: {}", error_value, error_str);
    }
#endif
}

} // namespace mmotd::error::posix_error
