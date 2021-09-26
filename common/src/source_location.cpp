// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/source_location.h"
#include "common/include/source_location_common.h"

#include <ostream>

#include <fmt/format.h>

using namespace std;

namespace mmotd::source_location {

string to_string(const SourceLocation &location) {
    auto function_name =
        TrimFunction(location.function_name(), FunctionArgStrategy::replace, FunctionReturnStrategy::remove);
    auto line_number = location.line();
    auto result = TrimFileName(location.file_name());
    if (!empty(result)) {
        result += '@';
    }
    result += function_name;
    if (line_number != 0) {
        result += string{"#"} + ::to_string(line_number);
    }
    return result;
}

ostream &operator<<(ostream &out, const SourceLocation &source_location) {
    out << to_string(source_location);
    return out;
}

} // namespace mmotd::source_location
