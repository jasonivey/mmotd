// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/tty_template_substring.h"
#include "common/include/tty_template_substrings.h"

#include <algorithm>
#include <iterator>
#include <string>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/range_fwd.hpp>
#include <fmt/color.h>
#include <plog/Log.h>

using namespace std;
using fmt::format;

namespace mmotd::tty_template::tty_string {

bool TemplateSubstrings::empty() const noexcept {
    return template_substrings_.empty();
}

TemplateSubstrings::size_type TemplateSubstrings::size() const noexcept {
    return template_substrings_.size();
}

TemplateSubstrings::reference TemplateSubstrings::front() noexcept {
    return *template_substrings_.front();
}

TemplateSubstrings::const_reference TemplateSubstrings::front() const noexcept {
    return *template_substrings_.front();
}

TemplateSubstrings::reference TemplateSubstrings::back() noexcept {
    return *template_substrings_.back();
}

TemplateSubstrings::const_reference TemplateSubstrings::back() const noexcept {
    return *template_substrings_.back();
}

TemplateSubstrings::iterator TemplateSubstrings::begin() {
    return boost::make_indirect_iterator(std::begin(template_substrings_));
}

TemplateSubstrings::const_iterator TemplateSubstrings::begin() const {
    return boost::make_indirect_iterator(std::cbegin(template_substrings_));
}

TemplateSubstrings::const_iterator TemplateSubstrings::cbegin() const {
    return boost::make_indirect_iterator(std::cbegin(template_substrings_));
}

TemplateSubstrings::reverse_iterator TemplateSubstrings::rbegin() {
    return boost::make_indirect_iterator(std::rbegin(template_substrings_));
}

TemplateSubstrings::const_reverse_iterator TemplateSubstrings::rbegin() const {
    return boost::make_indirect_iterator(std::crbegin(template_substrings_));
}

TemplateSubstrings::const_reverse_iterator TemplateSubstrings::crbegin() const {
    return boost::make_indirect_iterator(std::crbegin(template_substrings_));
}

TemplateSubstrings::iterator TemplateSubstrings::end() {
    return boost::make_indirect_iterator(std::end(template_substrings_));
}

TemplateSubstrings::const_iterator TemplateSubstrings::end() const {
    return boost::make_indirect_iterator(std::cend(template_substrings_));
}

TemplateSubstrings::const_iterator TemplateSubstrings::cend() const {
    return boost::make_indirect_iterator(std::cend(template_substrings_));
}

TemplateSubstrings::reverse_iterator TemplateSubstrings::rend() {
    return boost::make_indirect_iterator(std::rend(template_substrings_));
}

TemplateSubstrings::const_reverse_iterator TemplateSubstrings::rend() const {
    return boost::make_indirect_iterator(std::crend(template_substrings_));
}

TemplateSubstrings::const_reverse_iterator TemplateSubstrings::crend() const {
    return boost::make_indirect_iterator(std::crend(template_substrings_));
}

string TemplateSubstrings::to_string() const {
    auto output = string{};
    for_each(::cbegin(*this), ::cend(*this), [&output](const auto &template_substring) {
        output += template_substring.to_string();
    });
    return output;
}

string TemplateSubstrings::to_string(function<fmt::text_style(string)> convert_color) const {
    auto output = string{};
    auto i = size_t{0};
    for (const auto &template_substring : *this) {
        auto substring_output = template_substring.to_string(convert_color);
        PLOG_VERBOSE << format("{}: formatted text=\"{}\"", i, substring_output);
        output += substring_output;
    }
    return output;
}

} // namespace mmotd::tty_template::tty_string
