// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
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
    if (fs::is_regular_file(fortune_path, ec)) {
        PLOG_VERBOSE << format("file exists: {}", fortune_path.string());
    } else if (ec) {
        PLOG_ERROR << format("error code {} when calling is regular file for {}", ec.message(), fortune_path.string());
        return nullopt;
    } else {
        PLOG_ERROR << format("file does not exist: {}", fortune_path.string());
        return nullopt;
    }

    auto fortune_db_path = (fortune_path.parent_path() / fortune_path.stem()).replace_extension(".dat");
    if (fs::is_regular_file(fortune_db_path, ec)) {
        PLOG_VERBOSE << format("file exists: {}", fortune_db_path.string());
    } else if (ec) {
        PLOG_ERROR << format("error code {} when calling is regular file for {}",
                             ec.message(),
                             fortune_db_path.string());
        return nullopt;
    } else {
        PLOG_ERROR << format("file does not exist: {}", fortune_db_path.string());
        return nullopt;
    }

    return make_optional(make_tuple(fortune_path, fortune_db_path));
}

vector<uint32_t> ParseFortuneDbData(const vector<uint8_t> &buffer) {
    auto file_offsets = vector<uint32_t>{};
    auto i = begin(buffer);
    while (i != end(buffer)) {
        if (std::distance(i, end(buffer)) < static_cast<int32_t>(sizeof(uint32_t))) {
            PLOG_ERROR << format("found end and it was after file offset #{} and it's null uint32_t",
                                 file_offsets.size());
            i = end(buffer);
            continue;
        }

        auto raw_offset_ptr = reinterpret_cast<const uint32_t *>(&(*i));
        auto raw_offset = *raw_offset_ptr;
        file_offsets.emplace_back(ntohl(raw_offset));
        std::advance(i, sizeof(uint32_t));

        if (std::distance(i, end(buffer)) < static_cast<int32_t>(sizeof(uint32_t))) {
            PLOG_ERROR << format("found end and it was after file offset #{} but before it's null uint32_t",
                                 file_offsets.size());
            i = end(buffer);
            continue;
        }

        raw_offset_ptr = reinterpret_cast<const uint32_t *>(&(*i));
        raw_offset = *raw_offset_ptr;
        if (raw_offset != 0) {
            auto error_str = format(
                "unexpected input in strfile offsets table, the uint32_t following an offset is not 0 (value={}, index={}",
                raw_offset,
                std::distance(begin(buffer), i));
            PLOG_ERROR << error_str;
            return vector<uint32_t>{};
        }

        std::advance(i, sizeof(uint32_t));
    }

    //auto index = size_t{0};
    //for (auto offset : file_offsets) {
    //    PLOG_VERBOSE << format("file offset #{:3d}: offset value: {}", ++index, offset);
    //}

    return file_offsets;
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
    auto fortune_db_file = ifstream(fortune_db_path, ios_base::in | ios_base::binary);
    if (!fortune_db_file.is_open()) {
        PLOG_ERROR << format("unable to open {} for reading", fortune_db_path.string());
        return nullopt;
    }

    auto strfile_header = StrFileHeader{};
    fortune_db_file.read(reinterpret_cast<char *>(&strfile_header), sizeof(StrFileHeader));
    strfile_header.update();
    if (strfile_header.version != StrFileHeader::VERSION) {
        PLOG_ERROR << format("STRFILE header is version {} not the expected version {}",
                             strfile_header.version,
                             StrFileHeader::VERSION);
        return nullopt;
    }

    PLOG_VERBOSE << format("sizeof STRFILE header: {}", sizeof(StrFileHeader));
    auto remaining_size = static_cast<size_t>(fortune_db_file_size) - sizeof(StrFileHeader);
    PLOG_VERBOSE << format("remaining size: {}", remaining_size);
    PLOG_VERBOSE << format("number of strs: {}", strfile_header.numstr);

    auto buffer_size = static_cast<size_t>(strfile_header.numstr) * sizeof(uint64_t);
    auto buffer = vector<uint8_t>(buffer_size, 0);
    fortune_db_file.read(reinterpret_cast<char *>(buffer.data()), buffer_size);
    PLOG_VERBOSE << format("read the entire fortune db, buffer size: {}", buffer.size());

    auto file_offsets = ParseFortuneDbData(buffer);
    auto random_iterator = effing_random::get(file_offsets);
    PLOG_VERBOSE << format("random offset: {}", *random_iterator);
    return make_optional(make_tuple(*random_iterator, strfile_header.longlen, strfile_header.get_delim()));
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
            "updating quote read size (offset={} + read size={}) since it is now past eof (file size={} bytes)",
            fortune_offset,
            max_fortune_size,
            fortune_file_size);
        max_fortune_size = fortune_file_size - fortune_offset;
        PLOG_DEBUG << format(
            "updated quote read size (offset={} + read size={}) should now be to the eof (file size={} bytes)",
            fortune_offset,
            max_fortune_size,
            fortune_file_size);
    }

    auto fortune_file = ifstream(fortune_path, ios_base::in);
    if (!fortune_file.is_open()) {
        PLOG_ERROR << format("unable to open {} for reading", fortune_path.string());
        return nullopt;
    }

    auto buffer = vector<char>(max_fortune_size, 0);
    fortune_file.seekg(fortune_offset);
    fortune_file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    auto fortune = string{buffer.data()};
    auto i = fortune.find(fortune_delimeter);
    if (i != string::npos) {
        fortune = fortune.substr(0, i);
    }
    PLOG_VERBOSE << format("fortune:\n{}", fortune);
    return make_optional(trim_right_copy(fortune));
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
