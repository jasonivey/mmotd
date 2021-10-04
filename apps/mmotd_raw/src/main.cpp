// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd_raw/include/main.h"
#include "common/assertion/include/throw.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"

#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <regex>

#include <backward.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

using mmotd::algorithms::unused;
using namespace fmt;
using namespace std;

namespace {

string GetInformationNameString(const mmotd::information::Information &information) {
    const auto &name = information.GetName();
    static const auto formatting_color = fg(color::lime_green) | emphasis::bold;
    return !empty(name) ? format(formatting_color, FMT_STRING("{}"), name) + ": " : string{};
}

size_t GetInformationNameStringSize(const mmotd::information::Information &information) {
    return size(GetInformationNameString(information));
}

string GetInformationValueString(const mmotd::information::Information &information) {
    const auto &value = information.GetValue();
    static const auto formatting_color = fg(color::white) | emphasis::bold;
    return !empty(value) ? format(formatting_color, FMT_STRING("{}"), value) : string{};
}

size_t GetColumnWidth(const mmotd::information::Informations &informations) {
    auto i = max_element(begin(informations), end(informations), [](const auto &a, const auto &b) {
        return GetInformationNameStringSize(a) < GetInformationNameStringSize(b);
    });
    return i != end(informations) ? GetInformationNameStringSize(*i) : 0;
}

void PrintMmotdRaw() {
    auto &computer_information = mmotd::information::ComputerInformation::Instance();
    const auto &informations = computer_information.GetAllInformation();
    const auto width = GetColumnWidth(informations);
    for (const auto &information : informations) {
        auto name = GetInformationNameString(information);
        auto value = GetInformationValueString(information);
        if (!empty(name) && !empty(value)) {
            if (width > 0) {
                print(FMT_STRING("  {:<{}} {}\n"), name, width, value);
            } else {
                print(FMT_STRING("  {}{}\n"), name, value);
            }
        } else if (!empty(value)) {
            print(FMT_STRING("{}\n"), value);
        }
    }
}

int main_impl(int, char **argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    auto signal_handling = backward::SignalHandling{};
    unused(signal_handling);

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
        LOG_FATAL("{}", exception_message);
        retval = EXIT_FAILURE;
    }
    return retval;
}
