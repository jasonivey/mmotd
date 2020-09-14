#include "lastlog.h"
#include "network.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    auto last_log = LastLog{};
    auto last_login_data = LastLoginData{};
    if (!last_log.GetLastLoginRecord(last_login_data)) {
        cerr << "ERROR: unable to query for the last login record\n";
        EXIT_FAILURE;
    }
    if (!NetworkInfo().QueryForNetworkInfo()) {
        cerr << "ERROR: unable to query for IP and mac address\n";
        EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
