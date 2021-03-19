// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/results/include/template_substring_range.h"

#include <iterator>
#include <optional>
#include <string>

using namespace std;

namespace mmotd::results {

bool operator==(const SubstringRange &a, const SubstringRange &b) {
    return a.position_ == b.position_ && a.count_ == b.count_;
}

SubstringRange::SubstringRange(size_t position, size_t count) : position_(position), count_(count) {
}

SubstringRange::SubstringRange(string::const_iterator begin,
                               string::const_iterator substr_begin,
                               string::const_iterator substr_end) :
    position_(static_cast<size_t>(distance(begin, substr_begin))),
    count_(static_cast<size_t>(distance(substr_begin, substr_end))) {
}

optional<string> SubstringRange::substr(const string &txt) const {
    return IsValidSubstr(txt) ? make_optional(txt.substr(position_, count_)) : nullopt;
}

bool SubstringRange::IsValidSubstr(const string &txt) const {
    return !std::empty(txt) && !empty() && position_ < std::size(txt) && position_ + count_ <= std::size(txt);
}

} // namespace mmotd::results
