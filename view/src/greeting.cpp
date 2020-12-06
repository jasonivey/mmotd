// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/computer_information.h"
#include "view/include/computer_information_provider_factory.h"
#include "view/include/greeting.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using fmt::format;
using namespace std;

bool gLinkGreetingProvider = false;

static const bool factory_registered =
    mmotd::RegisterComputerInformationProvider([]() { return make_unique<mmotd::Greeting>(); });

namespace detail {

class GreetingGenerator {
public:
    GreetingGenerator() = default;

    void operator()(const string &info) {
        auto i = find_if(begin(component_names_), end(component_names_), [&info](const auto &name) {
            return boost::starts_with(info, format("{}: ", name));
        });
        if (i == end(component_names_)) {
            return;
        }
        const auto &name = *i;
        const auto &value = info.substr(string{format("{}: ", name)}.size());
        components_.emplace_back(make_tuple(name, value));
    }

    string to_string() const {
        // "Welcome to {platform name} {platform version} ({kernel type} {kernel release} {architecture})"
        return format("Welcome to {} {} ({} {} {})",
                      GetComponent("platform name"),
                      GetComponent("platform version"),
                      GetComponent("kernel type"),
                      GetComponent("kernel release"),
                      GetComponent("architecture"));
    }

private:
    string GetComponent(const string &name) const {
        for (const auto &component : components_) {
            const auto &[component_name, component_value] = component;
            if (component_name == name) {
                return component_value;
            }
        }
        return string{"unknown"};
    }
    array<string, 5> component_names_ = {"platform name",
                                         "platform version",
                                         "kernel release",
                                         "kernel type",
                                         "architecture"};
    vector<tuple<string, string>> components_;
};

} // namespace detail

optional<string> mmotd::Greeting::QueryInformation() {
    auto system_info_wrapper = ComputerInformation::Instance().GetInformation("system information");
    if (!system_info_wrapper) {
        PLOG_ERROR << "no host names were returned from posix system information";
        return nullopt;
    }
    auto system_infos = (*system_info_wrapper);

    auto greeting_generator = detail::GreetingGenerator{};
    for_each(begin(system_infos), end(system_infos), [&greeting_generator](const auto &system_info) {
        greeting_generator(system_info);
    });

    return make_optional(greeting_generator.to_string());
}

string mmotd::Greeting::GetName() const {
    return "greeting";
}
