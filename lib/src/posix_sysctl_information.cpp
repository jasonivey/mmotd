// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/posix_sysctl_information.h"

//#include <stdexcept>
//#include <sys/utsname.h>

//#include <boost/algorithm/string.hpp>
//#include <boost/log/trivial.hpp>
//#include <fmt/format.h>
#include <ostream>

//using namespace fmt;
using namespace std;

ostream &operator<<(ostream &out, const PosixSysctlInformation &sysctl_info) {
    (void) sysctl_info;
    return out;
}

string PosixSysctlInformation::to_string() const {
    return string{};
}

bool PosixSysctlInformation::TryDiscovery() {
    return true;
}
