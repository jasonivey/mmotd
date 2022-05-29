// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd_raw/include/main.h"

#include "common/assertion/include/assertion.h"
#include "common/assertion/include/throw.h"
#include "common/include/algorithm.h"
#include "common/include/global_state.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"

#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>

#include <backward.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

using mmotd::algorithms::unused;
using namespace std;

namespace {

string GetInformationNameString(const mmotd::information::Information &information) {
    const auto &name = information.GetName();
    static const auto formatting_color = fmt::fg(fmt::color::lime_green) | fmt::emphasis::bold;
    return !empty(name) ? fmt::format(formatting_color, FMT_STRING("{}: "), name) : string{};
}

size_t GetInformationNameStringSize(const mmotd::information::Information &information) {
    return size(GetInformationNameString(information));
}

string GetInformationValueString(const mmotd::information::Information &information) {
    const auto &value = information.GetValue();
    static const auto formatting_color = fmt::fg(fmt::color::white) | fmt::emphasis::bold;
    return !empty(value) ? fmt::format(formatting_color, FMT_STRING("{}"), value) : string{};
}

size_t GetColumnWidth(const vector<mmotd::information::Information> &informations) {
    auto i = max_element(begin(informations), end(informations), [](const auto &a, const auto &b) {
        return GetInformationNameStringSize(a) < GetInformationNameStringSize(b);
    });
    return i != end(informations) ? GetInformationNameStringSize(*i) : 0;
}

void PrintMmotdRaw() {
    auto &computer_information = mmotd::information::ComputerInformation::Instance();
    auto informations = computer_information.GetInformations();
    auto output_lines = vector<string>{};
    const auto width = GetColumnWidth(informations);
    for (const auto &information : informations) {
        auto name = GetInformationNameString(information);
        auto value = GetInformationValueString(information);
        if (!empty(name) && !empty(value)) {
            if (width > 0) {
                output_lines.push_back(fmt::format(FMT_STRING("  {:<{}} {}\n"), name, width, value));
            } else {
                output_lines.push_back(fmt::format(FMT_STRING("  {}{}\n"), name, value));
            }
        } else if (!empty(value)) {
            output_lines.push_back(fmt::format(FMT_STRING("{}\n"), value));
        }
    }
    sort(begin(output_lines), end(output_lines));
    copy(begin(output_lines), end(output_lines), ostream_iterator<string>(cout));
}

int main_impl(int, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto program_name = argv != nullptr && *argv != nullptr ? string_view(*argv) : string_view{};
    auto initilized = mmotd::logging::InitializeLogging(program_name);
    CHECKS(initilized, "unable to initialize logging");

    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);

    auto global_state = mmotd::globals::GlobalState{};
    unused(global_state);

    mmotd::logging::InitializeLogging(*argv);

    PrintMmotdRaw();
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char *argv[]) {
    auto retval = EXIT_SUCCESS;
    auto exception_message = string{};

    try {
        retval = main_impl(argc, argv);
    } catch (boost::exception &ex) {
        exception_message = mmotd::assertion::GetBoostExceptionMessage(ex);
    } catch (const std::exception &ex) {
        exception_message = mmotd::assertion::GetStdExceptionMessage(ex);
    } catch (...) { exception_message = mmotd::assertion::GetUnknownExceptionMessage(); }

    if (!empty(exception_message)) {
        fmt::print(stderr, FMT_STRING("{}\n"), exception_message);
        retval = EXIT_FAILURE;
    }
    return retval;
}
