// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/source_location.h"
#include "common/include/source_location_common.h"

#include <fmt/format.h>

using namespace std;

namespace mmotd::source_location {

SourceLocation::SourceLocation(const char *file, long line, const char *function) :
    line_(static_cast<uint32_t>(line)),
    // column_(0ul),
    file_name_(TrimFileName(file)),
    function_name_(TrimFunction(function, FunctionArgStrategy::replace, FunctionReturnStrategy::remove)) {
}

// 2021-04-22 22:19:30.397 VERB  [412078] [output_frame.cpp@mmotd::results::Frame::RemoveEmptyRows#179] inspecting row: 68646671, empty rows: 68646651
// to_string returns: `[output_frame.cpp@mmotd::results::Frame::RemoveEmptyRows#179]` in the best case scenario
//           returns: `[]` in the worst case scenario
string SourceLocation::to_string() const {
    auto result = string{};
    result = file_name_;
    result = format(FMT_STRING("{}{}{}"), result, (empty(result) ? "" : "@"), function_name_);
    result = format(FMT_STRING("{}{}{}"), result, (line_ != 0 ? "#" : ""), (line_ != 0 ? ::to_string(line_) : ""));
    return result;
}

string to_string(const SourceLocation &source_location) {
    return source_location.to_string();
}

} // namespace mmotd::source_location
