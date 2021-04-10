// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

namespace mmotd::information {

enum class InformationId : size_t;
class Information;

class Informations {
public:
    using value_type = Information;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = std::vector<Information>::iterator;
    using const_iterator = std::vector<Information>::const_iterator;
    using reverse_iterator = std::vector<Information>::reverse_iterator;
    using const_reverse_iterator = std::vector<Information>::const_reverse_iterator;

    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(Informations);

    Informations(std::initializer_list<Information> init_list) : informations_(init_list) {}

    void emplace_back(Information &&information);
    void push_back(const Information &information);

    bool empty() const noexcept;
    size_type size() const noexcept;
    size_type count(InformationId id) const;

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

private:
    std::vector<Information> informations_;
};

} // namespace mmotd::information
