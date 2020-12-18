// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/iostream_error.h"
#include "lib/include/computer_information.h"
#include "lib/include/fortune.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

#include <arpa/inet.h>

#include <effolkronium/random.hpp>

namespace fs = std::filesystem;
using effing_random = effolkronium::random_static;
using fmt::format;
using namespace std;
using boost::trim_right_copy;

bool gLinkFortuneGenerator = false;

namespace mmotd {

static const bool fortune_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::Fortune>(); });

bool Fortune::QueryInformation() {
    static bool has_queried = false;
    if (!has_queried) {
        has_queried = true;
        return GetFortune();
    }
    return has_queried;
}

optional<mmotd::ComputerValues> Fortune::GetInformation() const {
    return !details_.empty() ? make_optional(details_) : nullopt;
}

namespace detail {

#if defined(__APPLE__)
string GetPlatformFortunesPath() {
    return string{"/usr/local/opt/fortune/share/games/fortunes"};
}
#elif defined(__linux__)
string GetPlatformFortunesPath() {
    return string{"/usr/share/games/fortunes"};
}
#else
#    error no platform fortune path implemented
#endif

struct StrFileHeader { // information table
    static constexpr const unsigned long VERSION = 1;
    enum class Flags : unsigned long {
        RANDOM = 0x01,   // randomized pointers
        ORDEREDD = 0x02, // ordered pointers
        ROTATED = 0x04   // rot-13'd text
    };
    unsigned long version = 0;               // version number
    unsigned long numstr = 0;                // # of strings in the file
    unsigned long longlen = 0;               // length of longest string
    unsigned long shortlen = 0;              // length of shortest string
    unsigned long flags = 0;                 // bit field for flags
    unsigned char padding[4] = {0, 0, 0, 0}; // long aligned space

    char get_delim() const { return padding[0]; }

    void update() {
        version = ntohl(version);
        numstr = ntohl(numstr);
        longlen = ntohl(longlen);
        shortlen = ntohl(shortlen);
        flags = ntohl(flags);
    }
};

optional<tuple<fs::path, fs::path>> GetFortuneFiles(const string &fortune_name) {
    auto fortune_path = fs::path{fortune_name};
    if (fortune_name.find('/') == string::npos) {
        fortune_path = fs::path{GetPlatformFortunesPath()} / fortune_name;
    }

    auto ec = std::error_code{};
    if (fs::is_regular_file(fortune_path, ec) || fs::is_symlink(fortune_path, ec)) {
        PLOG_VERBOSE << format("file exists: {}", fortune_path.string());
    } else if (ec) {
        PLOG_ERROR << format("error {} when checking if file {} is regular/symlink",
                             ec.message(),
                             fortune_path.string());
        return nullopt;
    } else {
        PLOG_ERROR << format("file {} is not regular/symlink", fortune_path.string());
        return nullopt;
    }

    auto fortune_db_path = (fortune_path.parent_path() / fortune_path.stem()).replace_extension(".dat");
    if (fs::is_regular_file(fortune_db_path, ec) || fs::is_symlink(fortune_db_path, ec)) {
        PLOG_VERBOSE << format("file exists: {}", fortune_db_path.string());
    } else if (ec) {
        PLOG_ERROR << format("error {} when checking if file {} is regular/symlink",
                             ec.message(),
                             fortune_db_path.string());
        return nullopt;
    } else {
        PLOG_ERROR << format("file {} is not regular/symlink", fortune_db_path.string());
        return nullopt;
    }

    return make_optional(make_tuple(fortune_path, fortune_db_path));
}

optional<uint32_t> ParseSingleFortuneDbData(const vector<uint8_t> &buffer) {
    if (buffer.size() != sizeof(uint32_t) * 2) {
        PLOG_ERROR << format("unable to parse STRFILE when the input is not {}", sizeof(uint32_t) * 2);
        return nullopt;
    }

    auto network_offset_value = *reinterpret_cast<const uint32_t *>(buffer.data());
    auto host_offset_value = ntohl(network_offset_value);

    auto null_value = *reinterpret_cast<const uint32_t *>(buffer.data() + sizeof(uint32_t));
    if (null_value != 0) {
        auto error_str =
            format("STRFILE appears corrupt, uint32_t value {} is not followed by NULL uint32_t (uint32_t={})",
                   host_offset_value,
                   null_value);
        PLOG_ERROR << error_str;
        return nullopt;
    }
    PLOG_VERBOSE << format("parsed the STRFILE database entry, offset: {}", host_offset_value);
    return make_optional(host_offset_value);
}

optional<uint32_t>
ReadRandomFortuneOffset(const fs::path &fortune_db_path, std::ifstream &fortune_db_file, uint32_t file_offset) {
    PLOG_VERBOSE << format("seeking to offset {} in STRFILE database {}", file_offset, fortune_db_path.string());
    fortune_db_file.seekg(file_offset, std::ios_base::beg);
    if (!fortune_db_file.good()) {
        PLOG_ERROR << format("unable to seek to {} in STRFILE database {}, {}",
                             file_offset,
                             fortune_db_path.string(),
                             mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    auto buffer = vector<uint8_t>(sizeof(uint64_t), 0);
    fortune_db_file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    if (fortune_db_file.fail() || fortune_db_file.bad()) {
        PLOG_ERROR << format("unable to read {} bytes of STRFILE database {} at offset {}, {}",
                             sizeof(uint64_t),
                             fortune_db_path.string(),
                             file_offset,
                             mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    return ParseSingleFortuneDbData(buffer);
}

uint32_t ConvertDbIndexToFortuneFileOffset(size_t db_index, uint32_t file_size) {
    // adjust calculations for the STRFILE header and the following 2 uint32_t NULL's
    auto file_offset = sizeof(StrFileHeader) + sizeof(uint64_t);
    // add to the header size the actual database index * sizeof(uint64_t)
    file_offset += db_index * sizeof(uint64_t);
    // just make sure we haven't gone over the end of the file...
    return std::min(file_offset, file_size - sizeof(uint64_t));
}

optional<tuple<uint32_t, uint32_t, char>> GetRandomFortuneOffset(const fs::path &fortune_db_path) {
    auto ec = std::error_code{};
    auto fortune_db_file_size = fs::file_size(fortune_db_path, ec);
    if (ec) {
        PLOG_ERROR << format("error while getting file size of {}, details: {}",
                             fortune_db_path.string(),
                             ec.message());
        return nullopt;
    }
    auto fortune_db_file = ifstream{};
    fortune_db_file.exceptions(std::ifstream::goodbit);
    fortune_db_file.open(fortune_db_path, ios_base::in | ios_base::binary);

    if (!fortune_db_file.is_open() || fortune_db_file.fail() || fortune_db_file.bad()) {
        PLOG_ERROR << format("unable to open STRFILE database {} for reading, {}",
                             fortune_db_path.string(),
                             mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    auto strfile_header = StrFileHeader{};
    fortune_db_file.read(reinterpret_cast<char *>(&strfile_header), sizeof(StrFileHeader));
    if (fortune_db_file.fail() || fortune_db_file.bad()) {
        PLOG_ERROR << format("unable to read STRFILE database header {}, {}",
                             fortune_db_path.string(),
                             mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    strfile_header.update();
    if (strfile_header.version != StrFileHeader::VERSION) {
        PLOG_ERROR << format("STRFILE database header is version {} not the expected version {}",
                             strfile_header.version,
                             StrFileHeader::VERSION);
        return nullopt;
    }

    PLOG_VERBOSE << format("sizeof STRFILE header: {}", sizeof(StrFileHeader) + sizeof(uint64_t));
    auto remaining_size = static_cast<size_t>(fortune_db_file_size) - sizeof(StrFileHeader) - sizeof(uint64_t);
    PLOG_VERBOSE << format("remaining size : {}", remaining_size);
    PLOG_VERBOSE << format("number of strs : {}", strfile_header.numstr);
    PLOG_VERBOSE << format("calculated strs: {} with {} bytes remaining",
                           remaining_size / sizeof(uint64_t),
                           remaining_size % sizeof(uint64_t));

    auto random_index = effing_random::get<size_t>(0, static_cast<size_t>(strfile_header.numstr));
    PLOG_VERBOSE << format("random STRFILE database index: {}", random_index);
    auto file_offset = ConvertDbIndexToFortuneFileOffset(random_index, fortune_db_file_size);

    auto fortune_offset = ReadRandomFortuneOffset(fortune_db_path, fortune_db_file, file_offset);
    if (!fortune_offset) {
        return nullopt;
    }

    return make_optional(make_tuple(*fortune_offset, strfile_header.longlen, strfile_header.get_delim()));
}

optional<string> ReadFortune(const fs::path &fortune_path, uint32_t offset, uint32_t max_size, char delimeter) {
    auto fortune_file = ifstream{};
    fortune_file.exceptions(std::ifstream::goodbit);
    fortune_file.open(fortune_path, ios_base::in);

    if (!fortune_file.is_open() || fortune_file.fail() || fortune_file.bad()) {
        PLOG_ERROR << format("unable to open {} for reading, {}",
                             fortune_path.string(),
                             mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    fortune_file.seekg(offset);
    if (!fortune_file.good()) {
        PLOG_ERROR << format("unable to seek {} to fortune at offset {}, {}",
                             fortune_path.string(),
                             offset,
                             mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    auto buffer = vector<char>(max_size, 0);
    fortune_file.read(buffer.data(), buffer.size());
    if (fortune_file.fail() || fortune_file.bad()) {
        PLOG_ERROR << format("unable to read {} bytes of the fortune file {} at offset {}, {}",
                             max_size,
                             fortune_path.string(),
                             offset,
                             mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    auto fortune = string{buffer.data()};
    auto i = fortune.find(delimeter);
    if (i != string::npos) {
        fortune = fortune.substr(0, i);
    }
    PLOG_VERBOSE << format("fortune:\n{}", fortune);
    return make_optional(trim_right_copy(fortune));
}

optional<string> GetRandomFortune(const string &fortune_name) {
    auto fortune_files_wrapper = GetFortuneFiles(fortune_name);
    if (!fortune_files_wrapper) {
        return nullopt;
    }

    auto [fortune_path, fortune_db_path] = *fortune_files_wrapper;

    auto fortune_offset_wrapper = GetRandomFortuneOffset(fortune_db_path);
    if (!fortune_offset_wrapper) {
        return nullopt;
    }
    auto [fortune_offset, max_fortune_size, fortune_delimeter] = *fortune_offset_wrapper;

    auto ec = std::error_code{};
    auto fortune_file_size = fs::file_size(fortune_path, ec);
    if (ec) {
        PLOG_ERROR << format("error while getting file size of {}, details: {}", fortune_path.string(), ec.message());
        return nullopt;
    }

    if (fortune_offset >= fortune_file_size) {
        PLOG_ERROR << format("the fortune offset={} is beyond the fortune file size={}",
                             fortune_offset,
                             fortune_file_size);
        return nullopt;
    } else if (fortune_offset + max_fortune_size > fortune_file_size) {
        PLOG_DEBUG << format(
            "updating fortune read size (offset={} + read size={}) since it is now past eof (file size={} bytes)",
            fortune_offset,
            max_fortune_size,
            fortune_file_size);
        max_fortune_size = fortune_file_size - fortune_offset;
        PLOG_DEBUG << format(
            "updated fortune read size (offset={} + read size={}) should now be to the eof (file size={} bytes)",
            fortune_offset,
            max_fortune_size,
            fortune_file_size);
    }

    return ReadFortune(fortune_path, fortune_offset, max_fortune_size, fortune_delimeter);
}

} // namespace detail

bool Fortune::GetFortune() {
    details_.clear();
    auto random_fortune_wrapper = detail::GetRandomFortune("softwareengineering");
    if (!random_fortune_wrapper) {
        return false;
    }

    auto fortune = *random_fortune_wrapper;
    details_.push_back(make_tuple("fortune", fortune));
    return !details_.empty();
}

} // namespace mmotd
