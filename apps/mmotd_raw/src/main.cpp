// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "apps/mmotd_raw/include/main.h"
#include "common/include/algorithm.h"
#include "common/include/logging.h"
#include "lib/include/computer_information.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/range/iterator.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

using namespace fmt;
using namespace std;

namespace {

string GetInformationNameString(const mmotd::information::Information &information) {
    const auto &name = information.GetName();
    return !empty(name) ? format(fg(color::lime_green) | emphasis::bold, "{}", name) + ": " : string{};
}

size_t GetInformationNameStringSize(const mmotd::information::Information &information) {
    return size(GetInformationNameString(information));
}

string GetInformationValueString(const mmotd::information::Information &information) {
    const auto &value = information.GetValue();
    return !empty(value) ? format(fg(color::white) | emphasis::bold, "{}", value) : string{};
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
    mmotd::algorithms::unused(width);
    for (const auto &information : informations) {
        auto name = GetInformationNameString(information);
        auto value = GetInformationValueString(information);
        if (!empty(name) && !empty(value)) {
            if (width > 0) {
                //print("  {:<18} {}\n", name, value);
                print("  {:<{}} {}\n", name, width, value);
            } else {
                print("  {}{}\n", name, value);
            }
        } else if (!empty(value)) {
            print("  {}\n", value);
        }
    }
}

} // namespace

int main(int argc, char *argv[]) {
    mmotd::algorithms::unused(argc, argv);
    mmotd::logging::DefaultInitializeLogging("mmotd_raw.log");

    auto retval = EXIT_SUCCESS;
    try {
        PrintMmotdRaw();
    } catch (boost::exception &ex) {
        auto diag = boost::diagnostic_information(ex);
        auto error_str = format("caught boost::exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (const std::exception &ex) {
        auto error_str = format("caught std::exception in main: {}", ex.what());
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    } catch (...) {
        auto diag = boost::current_exception_diagnostic_information();
        auto error_str = format("caught unknown exception in main: {}", diag);
        PLOG_FATAL << error_str;
        std::cerr << error_str << std::endl;
        retval = EXIT_FAILURE;
    }

    return retval;
}