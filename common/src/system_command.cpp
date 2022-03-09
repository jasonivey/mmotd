// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/logging.h"
#include "common/include/system_command.h"

#include <filesystem>
#include <iomanip>
#include <system_error>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <fmt/format.h>

using fmt::format;
namespace fs = std::filesystem;
using namespace std;
using namespace std::string_literals;

namespace {

string CreateCommand(const fs::path &exe_path, const vector<string> &args) {
    return format(FMT_STRING("{} {}"), exe_path.string(), fmt::join(args, " "));
}

string CreateErrorStr(int result, string command, string std_out, string std_err) {
    auto output = format(FMT_STRING("command: {} returned "), quoted(command));
    output += format(FMT_STRING("[{}] {}, "), result == 0 ? "SUCCESS"s : "ERROR"s, result);
    if (empty(std_out)) {
        output += string{"stdout: \"\", "};
    } else {
        output += format(FMT_STRING("stdout:\n{}\n"), std_out);
    }
    if (empty(std_err)) {
        output += string{"stderr: \"\""};
    } else {
        output += format(FMT_STRING("stderr:\n{}"), std_err);
    }
    return output;
}

} // namespace

namespace mmotd::system::command {

optional<string> Run(const fs::path &exe_path, const vector<string> &args) {
    namespace bp = boost::process;
    namespace io = boost::asio;
    if (empty(exe_path)) {
        return nullopt;
    }
    auto ec = error_code{};
    if (!fs::exists(exe_path, ec)) {
        LOG_ERROR("unable to execute '{}' since it does not exist", exe_path.string());
        return nullopt;
    }
    auto command = CreateCommand(exe_path, args);
    auto exit_code = std::future<int>{};
    auto data = std::future<std::string>{};
    auto error = std::future<std::string>{};
    auto io_service = io::io_service{};
    auto child_process = bp::child(command,
                                   bp::std_in.close(),
                                   bp::std_out > data,
                                   bp::std_err > error,
                                   bp::on_exit = exit_code,
                                   io_service);
    io_service.run();
    auto result = exit_code.get();
    auto std_output = data.get();
    auto std_error = error.get();

    if (result != 0 || empty(std_output)) {
        auto error_str = CreateErrorStr(result, command, std_output, std_error);
        LOG_ERROR("{}", error_str);
        return nullopt;
    } else {
        return {std_output};
    }
}

} // namespace mmotd::system::command
