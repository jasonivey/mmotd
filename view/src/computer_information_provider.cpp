// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/computer_information_provider.h"

using namespace std;

optional<string> mmotd::ComputerInformationProvider::GetComputerInformation() {
    return QueryInformation();
}

string mmotd::ComputerInformationProvider::GetInformationName() const {
    return GetName();
}
