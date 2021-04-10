// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <fmt/color.h>

namespace mmotd::results {

class TemplateSubstring;

class TemplateSubstrings {
    using SmartPointer = std::unique_ptr<TemplateSubstring>;
    using Container = std::vector<SmartPointer>;

public:
    using value_type = TemplateSubstring;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = boost::indirect_iterator<Container::iterator>;
    using const_iterator = boost::indirect_iterator<Container::const_iterator>;
    using reverse_iterator = boost::indirect_iterator<Container::reverse_iterator>;
    using const_reverse_iterator = boost::indirect_iterator<Container::const_reverse_iterator>;

    template<class... Args>
    void emplace_back(Args &&...args) {
        template_substrings_.emplace_back(std::forward<Args>(args)...);
    }

    bool empty() const noexcept;
    size_type size() const noexcept;

    reference front() noexcept;
    const_reference front() const noexcept;
    reference back() noexcept;
    const_reference back() const noexcept;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;
    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const;

    std::string to_string() const;
    std::string to_string(std::function<fmt::text_style(std::string)> convert_color) const;

private:
    Container template_substrings_;
};

} // namespace mmotd::results
