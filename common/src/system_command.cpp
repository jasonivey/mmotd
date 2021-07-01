// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"
#include "common/include/system_command.h"

#include <filesystem>
#include <system_error>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <fmt/format.h>

using fmt::format;
namespace fs = std::filesystem;
using namespace std;

namespace {

string CreateCommand(const fs::path &exe_path, const vector<string> &args) {
    return format(FMT_STRING("{} {}"), exe_path.string(), fmt::join(args, " "));
}

} // namespace

namespace mmotd::system::command {

optional<string> Run(const fs::path &exe_path, const vector<string> &args) {
    namespace bp = boost::process;
    namespace io = boost::asio;

    auto ec = error_code{};
    if (!fs::exists(exe_path, ec)) {
        LOG_ERROR("unable to execute '{}' since it does not exist", exe_path.string());
        return nullopt;
    }
    auto command = CreateCommand(exe_path, args);

    auto exit_code = std::future<int>{};
    auto data = std::future<std::string>{};
    auto io_service = io::io_service{};
    auto child_process = bp::child(command, bp::std_out > data, io_service, bp::on_exit = exit_code);
    io_service.run();
    auto result = exit_code.get();
    auto output = data.get();

    if (result != 0) {
        LOG_ERROR("'{}' returned error {}, output:\n{}", command, result, output);
        return nullopt;
    } else if (empty(output)) {
        LOG_ERROR("'{}' returned 0 [success], without any output", command);
        return nullopt;
    } else {
        return {output};
    }
}

} // namespace mmotd::system::command
