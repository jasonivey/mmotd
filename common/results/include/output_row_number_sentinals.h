// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <string>

namespace mmotd::results {

class RowNumberSentinals {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using value_type = int;
    using pointer = int *;
    using const_pointer = const int *;
    using reference = int &;
    using const_reference = const int &;

    RowNumberSentinals(value_type begin, value_type end);
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(RowNumberSentinals);

    value_type front() const noexcept { return begin_value_; }
    value_type back() const noexcept { return end_value_; }

    class iterator_state {
    public:
        inline void begin(const RowNumberSentinals *ref) { value = ref->begin_value_; }
        inline void end(const RowNumberSentinals *ref) { value = ref->end_value_ + 1; }
        inline void next(const RowNumberSentinals *) { ++value; }
        inline void prev(const RowNumberSentinals *) { --value; }
        inline int &get(RowNumberSentinals *) { return value; }
        inline const int &get(const RowNumberSentinals *) const { return value; }
        inline bool equal_to(const iterator_state &state) const { return value == state.value; }

    private:
        int value = 0;
    };

    class const_iterator;

    class iterator {
        friend class const_iterator;

    public:
        iterator() = default;
        void begin();
        void end();
        void next();
        void prev();
        int &get();
        bool equal_to(const iterator_state &other_state) const;

        static iterator begin(RowNumberSentinals *ref);
        static iterator end(RowNumberSentinals *ref);

        int operator*();
        iterator &operator++();
        iterator operator++(int);
        iterator &operator--();
        iterator operator--(int);
        bool operator!=(const iterator &other) const;
        bool operator==(const iterator &other) const;
        bool operator!=(const const_iterator &other) const;
        bool operator==(const const_iterator &other) const;

    private:
        iterator(RowNumberSentinals *ref);

        RowNumberSentinals *ref_ = nullptr;
        iterator_state state;
    };

    class const_iterator {
        friend class iterator;

    public:
        const_iterator() = default;
        const_iterator(const iterator &other);

        void begin();
        void end();
        void next();
        void prev();
        const int &get();
        bool equal_to(const iterator_state &other_state) const;

        static const_iterator begin(const RowNumberSentinals *ref);
        static const_iterator end(const RowNumberSentinals *ref);

        const int &operator*();
        const_iterator &operator++();
        const_iterator operator++(int);
        const_iterator &operator--();
        const_iterator operator--(int);
        bool operator!=(const const_iterator &other) const;
        bool operator==(const const_iterator &other) const;
        const_iterator &operator=(const iterator &other);
        bool operator!=(const iterator &other) const;
        bool operator==(const iterator &other) const;

    private:
        const_iterator(const RowNumberSentinals *ref);

        const RowNumberSentinals *ref_ = nullptr;
        iterator_state state;
    };

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    friend std::ostream &operator<<(std::ostream &out, const RowNumberSentinals &sentinals);
    std::string to_string() const;

private:
    int begin_value_ = -1;
    int end_value_ = -1;
};

} // namespace mmotd::results
