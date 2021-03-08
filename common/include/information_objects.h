// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"

#include <iterator>
#include <vector>

namespace mmotd::information {

// for now this forward declaration is keeping from including information_definitions.h
class Information;
class Informations;

class InformationObjects {
public:
    DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(InformationObjects);

    static InformationObjects &Instance();

    void AddObject(Information &&information);
    void AddObject(const Information &information);

    auto begin() { return objects_.begin(); }
    auto cbegin() const { return objects_.cbegin(); }
    auto end() { return objects_.end(); }
    auto cend() const { return objects_.cend(); }

private:
    Informations objects_;
};

} // namespace mmotd::information
