// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/results/include/output_row_number_sentinals.h"

#include <limits>
#include <ostream>
#include <string>

#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

static constexpr const int INVALID_ROW = std::numeric_limits<int>::max();

namespace mmotd::results {

RowNumberSentinals::RowNumberSentinals(value_type begin, value_type end) : begin_value_(begin), end_value_(end) {
    PLOG_VERBOSE << format("row sentinal created: {}", to_string());
}

RowNumberSentinals::iterator RowNumberSentinals::begin() {
    return iterator::begin(this);
}

RowNumberSentinals::iterator RowNumberSentinals::end() {
    return iterator::end(this);
}

RowNumberSentinals::const_iterator RowNumberSentinals::begin() const {
    return const_iterator::begin(this);
}

RowNumberSentinals::const_iterator RowNumberSentinals::end() const {
    return const_iterator::end(this);
}

RowNumberSentinals::const_iterator RowNumberSentinals::cbegin() const {
    return const_iterator::begin(this);
}

RowNumberSentinals::const_iterator RowNumberSentinals::cend() const {
    return const_iterator::end(this);
}

RowNumberSentinals::iterator::iterator(RowNumberSentinals *ref) : ref_(ref), state() {
}

void RowNumberSentinals::iterator::begin() {
    state.begin(ref_);
}

void RowNumberSentinals::iterator::end() {
    state.end(ref_);
}

void RowNumberSentinals::iterator::next() {
    state.next(ref_);
}

void RowNumberSentinals::iterator::prev() {
    state.prev(ref_);
}

int &RowNumberSentinals::iterator::get() {
    return state.get(ref_);
}

bool RowNumberSentinals::iterator::equal_to(const iterator_state &other_state) const {
    return state.equal_to(other_state);
}

RowNumberSentinals::iterator RowNumberSentinals::iterator::begin(RowNumberSentinals *ref) {
    iterator i(ref);
    i.begin();
    return i;
}

RowNumberSentinals::iterator RowNumberSentinals::iterator::end(RowNumberSentinals *ref) {
    iterator i(ref);
    i.end();
    return i;
}

int RowNumberSentinals::iterator::operator*() {
    return get();
}

RowNumberSentinals::iterator &RowNumberSentinals::iterator::operator++() {
    next();
    return *this;
}

RowNumberSentinals::iterator RowNumberSentinals::iterator::operator++(int) {
    auto current = iterator(*this);
    next();
    return current;
}

RowNumberSentinals::iterator &RowNumberSentinals::iterator::operator--() {
    prev();
    return *this;
}

RowNumberSentinals::iterator RowNumberSentinals::iterator::operator--(int) {
    auto current = iterator(*this);
    prev();
    return current;
}

bool RowNumberSentinals::iterator::operator!=(const RowNumberSentinals::iterator &other) const {
    return ref_ != other.ref_ || !state.equal_to(other.state);
}

bool RowNumberSentinals::iterator::operator==(const RowNumberSentinals::iterator &other) const {
    return !operator!=(other);
}

bool RowNumberSentinals::iterator::operator!=(const RowNumberSentinals::const_iterator &other) const {
    return ref_ != other.ref_ || !state.equal_to(other.state);
}

bool RowNumberSentinals::iterator::operator==(const RowNumberSentinals::const_iterator &other) const {
    return !operator!=(other);
}

RowNumberSentinals::const_iterator::const_iterator(const iterator &other) : ref_(other.ref_), state(other.state) {
}

RowNumberSentinals::const_iterator::const_iterator(const RowNumberSentinals *ref) : ref_(ref), state() {
}

void RowNumberSentinals::const_iterator::begin() {
    state.begin(ref_);
}

void RowNumberSentinals::const_iterator::end() {
    state.end(ref_);
}

void RowNumberSentinals::const_iterator::next() {
    state.next(ref_);
}

void RowNumberSentinals::const_iterator::prev() {
    state.prev(ref_);
}

const int &RowNumberSentinals::const_iterator::get() {
    return state.get(ref_);
}

bool RowNumberSentinals::const_iterator::equal_to(const iterator_state &other_state) const {
    return state.equal_to(other_state);
}

RowNumberSentinals::const_iterator RowNumberSentinals::const_iterator::begin(const RowNumberSentinals *ref) {
    auto i = const_iterator(ref);
    i.begin();
    return i;
}

RowNumberSentinals::const_iterator RowNumberSentinals::const_iterator::end(const RowNumberSentinals *ref) {
    auto i = const_iterator(ref);
    i.end();
    return i;
}

const int &RowNumberSentinals::const_iterator::operator*() {
    return get();
}

RowNumberSentinals::const_iterator &RowNumberSentinals::const_iterator::operator++() {
    next();
    return *this;
}

RowNumberSentinals::const_iterator RowNumberSentinals::const_iterator::operator++(int) {
    auto current = const_iterator(*this);
    next();
    return current;
}

RowNumberSentinals::const_iterator &RowNumberSentinals::const_iterator::operator--() {
    prev();
    return *this;
}

RowNumberSentinals::const_iterator RowNumberSentinals::const_iterator::operator--(int) {
    auto current = const_iterator(*this);
    prev();
    return current;
}

bool RowNumberSentinals::const_iterator::operator!=(const RowNumberSentinals::const_iterator &other) const {
    return ref_ != other.ref_ || !equal_to(other.state);
}

bool RowNumberSentinals::const_iterator::operator==(const RowNumberSentinals::const_iterator &other) const {
    return !operator!=(other);
}

RowNumberSentinals::const_iterator &
RowNumberSentinals::const_iterator::operator=(const RowNumberSentinals::iterator &other) {
    ref_ = other.ref_;
    state = other.state;
    return *this;
}

bool RowNumberSentinals::const_iterator::operator!=(const RowNumberSentinals::iterator &other) const {
    return ref_ != other.ref_ || !equal_to(other.state);
}

bool RowNumberSentinals::const_iterator::operator==(const RowNumberSentinals::iterator &other) const {
    return !operator!=(other);
}

string RowNumberSentinals::to_string() const {
    if (begin_value_ == INVALID_ROW || end_value_ == INVALID_ROW) {
        return string{"{INVALID : INVALID}"};
    } else {
        return format("{{{} : {}}}", begin_value_, end_value_);
    }
}

ostream &operator<<(ostream &out, const RowNumberSentinals &sentinals) {
    out << sentinals.to_string();
    return out;
}

} // namespace mmotd::results
