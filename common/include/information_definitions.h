// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include "common/include/big_five_macros.h"
#include "common/include/information_decls.h"
#include "common/include/informations.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <mutex>
#include <string>
#include <tuple>

namespace mmotd::information {

enum class CategoryId : size_t;
enum class InformationId : size_t;

class InformationDefinitions {
public:
    ~InformationDefinitions() = default;

    static const InformationDefinitions &Instance();

    Information GetInformationDefinition(InformationId id) const;

private:
    InformationDefinitions(CategoryIds categories, Informations informations);
    InformationDefinitions() = default;

    CategoryIds categories_;
    Informations informations_;
    mutable std::mutex mutex_;
};

} // namespace mmotd::information
