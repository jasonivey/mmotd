// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/information.h"
#include "common/include/information_definitions.h"
#include "common/include/information_objects.h"
#include "common/include/informations.h"

namespace mmotd::information {

InformationObjects &InformationObjects::Instance() {
    static auto information_objects = InformationObjects{};
    return information_objects;
}

void InformationObjects::AddObject(Information &&information) {
    objects_.emplace_back(std::move(information));
}

void InformationObjects::AddObject(const Information &information) {
    objects_.push_back(information);
}

} // namespace mmotd::information
