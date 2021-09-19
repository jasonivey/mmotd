// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/logging.h"
#include "common/include/posix_error.h"
#include "common/include/user_information.h"

#include <cstring>
#include <string>
#include <vector>

#include <fmt/format.h>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using fmt::format;
using mmotd::core::UserInformation;
namespace pe = mmotd::error::posix_error;

namespace {

UserInformation GetUserInformationImpl() {
    auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) {
        bufsize = 16 * 1024;
    }
    auto buf = vector<char>(static_cast<size_t>(bufsize));
    auto pwd = passwd{};
    passwd *pwd_ptr = nullptr;
    auto user_id = geteuid();
    auto retval = getpwuid_r(user_id, &pwd, buf.data(), buf.size(), &pwd_ptr);
    if (pwd_ptr == nullptr && retval == 0) {
        LOG_ERROR("getpwnam_r for user id {} did not return a pwd structure or an error code", user_id);
        return UserInformation{};
    } else if (pwd_ptr == nullptr && retval != 0) {
        auto error_str = pe::to_string(retval);
        LOG_ERROR("getpwnam_r for user id {} failed, {}", user_id, error_str);
        return UserInformation{};
    }
    return UserInformation::from_passwd(pwd);
}

} // namespace

namespace mmotd::core {

UserInformation UserInformation::from_passwd(const passwd &pw) {
    auto username = strlen(pw.pw_name) > 0 ? string{pw.pw_name} : string{};
    auto user_id = pw.pw_uid;
    auto group_id = pw.pw_gid;
    auto full_name = strlen(pw.pw_gecos) > 0 ? string{pw.pw_gecos} : string{};
    auto home_directory = strlen(pw.pw_dir) > 0 ? string{pw.pw_dir} : string{};
    auto shell = strlen(pw.pw_shell) > 0 ? string{pw.pw_shell} : string{};
    return UserInformation{username, user_id, group_id, full_name, home_directory, shell};
}

const UserInformation &GetUserInformation() {
    static const auto user_information = GetUserInformationImpl();
    return user_information;
}

} // namespace mmotd::core
