// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information.h"
#include "common/include/informations.h"

#include <algorithm>
#include <iterator>
#include <string>

// #include <fmt/format.h>
// #include <fmt/ostream.h>
// #include <plog/Log.h>

//using fmt::format;
using namespace std;

namespace mmotd::information {

void Informations::emplace_back(Information &&information) {
    informations_.emplace_back(std::move(information));
}

void Informations::push_back(const Information &information) {
    informations_.push_back(information);
}

bool Informations::empty() const noexcept {
    return informations_.empty();
}

Informations::size_type Informations::size() const noexcept {
    return informations_.size();
}

Informations::size_type Informations::count(InformationId id) const {
    return count_if(informations_.begin(), informations_.end(), [id](const auto &info) {
        return info.information_id == id;
    });
}

Informations::reference Informations::front() noexcept {
    return informations_.front();
}

Informations::const_reference Informations::front() const noexcept {
    return informations_.front();
}

Informations::reference Informations::back() noexcept {
    return informations_.back();
}

Informations::const_reference Informations::back() const noexcept {
    return informations_.back();
}

Informations::iterator Informations::begin() {
    return std::begin(informations_);
}

Informations::const_iterator Informations::begin() const {
    return std::cbegin(informations_);
}

Informations::const_iterator Informations::cbegin() const {
    return std::cbegin(informations_);
}

Informations::reverse_iterator Informations::rbegin() {
    return std::rbegin(informations_);
}

Informations::const_reverse_iterator Informations::rbegin() const {
    return std::crbegin(informations_);
}

Informations::const_reverse_iterator Informations::crbegin() const {
    return std::crbegin(informations_);
}

Informations::iterator Informations::end() {
    return std::end(informations_);
}

Informations::const_iterator Informations::end() const {
    return std::cend(informations_);
}

Informations::const_iterator Informations::cend() const {
    return std::cend(informations_);
}

Informations::reverse_iterator Informations::rend() {
    return std::rend(informations_);
}

Informations::const_reverse_iterator Informations::rend() const {
    return std::crend(informations_);
}

Informations::const_reverse_iterator Informations::crend() const {
    return std::crend(informations_);
}

} // namespace mmotd::information
