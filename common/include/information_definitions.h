// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/information_decls.h"

#include <cstdlib> // for std::size_t
#include <vector>

namespace mmotd::information {

//
// This singleton is designed to be used by multiple threads.  The public interface shows
//  how the object is read-only (const correct) and is thread-safe:
//  1. The only data stored in the object is added during the construction of the
//     static singleton object.  C++11 (and beyond) formalized the thread safety of static
//     variables.  In short, if multiple threads access an uninitialized static variable
//     only one thread will be allowed to initialize the variable before access is
//     allowed to all other threads.
//  2. The only member function is a read-only query which returns copies of the
//     `Information` objects if `id`s match.
//  3. If that member function query fails it throws a `std::runtime_error`.
//

class InformationDefinitions {
public:
    ~InformationDefinitions() = default;

    static const InformationDefinitions &Instance();

    Information GetInformationDefinition(InformationId id) const;

private:
    InformationDefinitions(CategoryIds categories, std::vector<Information> informations);
    InformationDefinitions() = default;

    CategoryIds categories_;
    std::vector<Information> informations_;
};

} // namespace mmotd::information
