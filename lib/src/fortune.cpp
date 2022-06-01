// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "lib/include/fortune.h"

#include "common/include/algorithm.h"
#include "common/include/config_options.h"
#include "common/include/iostream_error.h"
#include "common/include/logging.h"
#include "common/include/special_files.h"
#include "lib/include/computer_information.h"

#include <array>
#include <bit>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <effolkronium/random.hpp>
#include <fmt/format.h>

#include <arpa/inet.h>

namespace fs = std::filesystem;
using effing_random = effolkronium::random_static;
using fmt::format;
using namespace std;
using boost::trim_right_copy;

bool gLinkFortuneGenerator = false;

namespace mmotd::information {

static const bool fortune_factory_registered =
    RegisterInformationProvider([]() { return make_unique<mmotd::information::Fortune>(); });

} // namespace mmotd::information

namespace {

#if defined(__APPLE__)

static constexpr uint32_t STRFILE_VERSION = 1;
using StrFileIntType = uint64_t;
static constexpr size_t STRFILE_HEADER_PADDING = sizeof(StrFileIntType);
static constexpr bool STRFILE_ENTRY_CONTAINS_NULL_INT = true;

string_view GetPlatformFortunesPath() {
    return "/usr/local/opt/fortune/share/games/fortunes";
}

#define FORTUNE_FILE_BYTESWAP(x) ntohll(x)

#elif defined(__linux__)

static constexpr uint32_t STRFILE_VERSION = 2;
using StrFileIntType = uint32_t;
static constexpr size_t STRFILE_HEADER_PADDING = 0;
static constexpr bool STRFILE_ENTRY_CONTAINS_NULL_INT = false;

string_view GetPlatformFortunesPath() {
    return "/usr/share/games/fortunes";
}

#define FORTUNE_FILE_BYTESWAP(x) ntohl(x)

#endif

static constexpr size_t STRFILE_ENTRY_SIZE = sizeof(StrFileIntType);

struct StrFileHeader {
    StrFileHeader() = default;

    enum class Flags : StrFileIntType {
        None = 0x00,
        Random = 0x01,  // randomized pointers
        Ordered = 0x02, // ordered pointers
        Rotated = 0x04  // rot-13'd text
    };

    friend constexpr Flags operator&(Flags a, Flags b) {
        return static_cast<Flags>(static_cast<StrFileIntType>(a) & static_cast<StrFileIntType>(b));
    }

    StrFileIntType version = 0;      // version number
    StrFileIntType count = 0;        // count of strings in the fortune file
    StrFileIntType longest_str = 0;  // length of longest fortune
    StrFileIntType shortest_str = 0; // length of shortest shortest fortune
    Flags flags = Flags::None;       // flags
    union DelimPadding {
        char delim;                // delimeter between fortunes
        array<uint8_t, 4> padding; // padding
    };
    DelimPadding delim_padding = {.padding = {0, 0, 0, 0}};

    char get_delim() const { return delim_padding.delim; }

    string flags_to_string() const {
        auto flags_str = vector<string>{};
        if (flags == Flags::None) {
            flags_str.emplace_back("none");
        }
        if ((flags & Flags::Random) != Flags::None) {
            flags_str.emplace_back("random");
        }
        if ((flags & Flags::Ordered) != Flags::None) {
            flags_str.emplace_back("ordered");
        }
        if ((flags & Flags::Rotated) != Flags::None) {
            flags_str.emplace_back("rotated");
        }
        return format(FMT_STRING("[{}]"), boost::join(flags_str, ", "));
    }

    string to_string() const {
        return format(FMT_STRING("version: {}, count: {}, longest: {}, shortest: {}, flags: {}, delim: {}"),
                      version,
                      count,
                      longest_str,
                      shortest_str,
                      flags_to_string(),
                      get_delim());
    }

    void update() {
        version = FORTUNE_FILE_BYTESWAP(version);
        count = FORTUNE_FILE_BYTESWAP(count);
        longest_str = FORTUNE_FILE_BYTESWAP(longest_str);
        shortest_str = FORTUNE_FILE_BYTESWAP(shortest_str);
        flags = static_cast<Flags>(FORTUNE_FILE_BYTESWAP(static_cast<StrFileIntType>(flags)));
        LOG_VERBOSE("STRFILE: {}", to_string());
    }
};

optional<tuple<fs::path, fs::path>> GetFortuneFiles(string fortune_filename, string fortune_db_dir) {
    auto fortune_path = fs::path{};
    if (empty(fortune_db_dir)) {
        fortune_path = fs::path{fortune_filename};
    } else {
        fortune_path = fs::path{fortune_db_dir} / fs::path{fortune_filename};
    }

    auto ec = std::error_code{};
    if (fs::is_regular_file(fortune_path, ec) || fs::is_symlink(fortune_path, ec)) {
        LOG_VERBOSE("file exists: {}", fortune_path.string());
    } else if (ec) {
        LOG_WARNING("checking if file {} is regular/symlink: '{}'", fortune_path.string(), ec.message());
        return nullopt;
    } else {
        LOG_ERROR("file {} is not regular/symlink", fortune_path.string());
        return nullopt;
    }

    auto fortune_db_path = fortune_path;
    fortune_db_path.replace_extension(".dat");
    if (fs::is_regular_file(fortune_db_path, ec) || fs::is_symlink(fortune_db_path, ec)) {
        LOG_VERBOSE("file exists: {}", fortune_db_path.string());
    } else if (ec) {
        LOG_ERROR("checking if file {} is regular/symlink: {}", fortune_db_path.string(), ec.message());
        return nullopt;
    } else {
        LOG_ERROR("file {} is not regular/symlink", fortune_db_path.string());
        return nullopt;
    }

    return make_optional(make_tuple(fortune_path, fortune_db_path));
}

optional<uint32_t> ParseSingleFortuneDbData(const array<char, STRFILE_ENTRY_SIZE> &buffer) {
    uint32_t network_offset_value = 0;
    memcpy(&network_offset_value, data(buffer), std::min(sizeof(uint32_t), size(buffer)));
    auto host_offset_value = ntohl(network_offset_value);

    if constexpr (STRFILE_ENTRY_CONTAINS_NULL_INT) {
        uint32_t null_value = 0;
        memcpy(&null_value, data(buffer) + sizeof(uint32_t), sizeof(uint32_t));
        if (null_value != 0) {
            LOG_ERROR("STRFILE appears corrupt, uint32_t value {} is not followed by NULL uint32_t (uint32_t={})",
                      host_offset_value,
                      null_value);
            return nullopt;
        }
    }

    LOG_VERBOSE("parsed the STRFILE database entry, offset: {}", host_offset_value);
    return make_optional(host_offset_value);
}

optional<uint32_t>
ReadRandomFortuneOffset(const fs::path &fortune_db_path, std::ifstream &fortune_db_file, uint32_t file_offset) {
    LOG_VERBOSE("seeking to offset {} in STRFILE database {}", file_offset, fortune_db_path.string());
    fortune_db_file.seekg(file_offset, std::ios_base::beg);
    if (!fortune_db_file.good()) {
        LOG_ERROR("unable to seek to {} in STRFILE database {}, {}",
                  file_offset,
                  fortune_db_path.string(),
                  mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    auto buffer = array<char, STRFILE_ENTRY_SIZE>{};
    fortune_db_file.read(data(buffer), static_cast<streamsize>(size(buffer)));
    if (fortune_db_file.fail() || fortune_db_file.bad()) {
        LOG_ERROR("unable to read {} bytes of STRFILE database {} at offset {}, {}",
                  STRFILE_ENTRY_SIZE,
                  fortune_db_path.string(),
                  file_offset,
                  mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    return ParseSingleFortuneDbData(buffer);
}

uint32_t ConvertDbIndexToFortuneFileOffset(size_t db_index, uint32_t file_size) {
    // adjust calculations for the STRFILE header and the following 2 uint32_t NULL's
    auto file_offset = sizeof(StrFileHeader) + STRFILE_HEADER_PADDING;
    // add to the header size the actual database index * sizeof(uint64_t)
    file_offset += db_index * STRFILE_ENTRY_SIZE;
    // just make sure we haven't gone over the end of the file...
    return static_cast<uint32_t>(std::min(file_offset, file_size - STRFILE_ENTRY_SIZE));
}

optional<tuple<uint32_t, uint32_t, char>> GetRandomFortuneOffset(const fs::path &fortune_db_path) {
    auto ec = std::error_code{};
    auto fortune_db_file_size = fs::file_size(fortune_db_path, ec);
    if (ec) {
        LOG_ERROR("error while getting file size of {}, details: {}", fortune_db_path.string(), ec.message());
        return nullopt;
    }
    auto fortune_db_file = ifstream{};
    fortune_db_file.exceptions(std::ifstream::goodbit);
    fortune_db_file.open(fortune_db_path, ios_base::in | ios_base::binary);

    if (!fortune_db_file.is_open() || fortune_db_file.fail() || fortune_db_file.bad()) {
        LOG_ERROR("unable to open STRFILE database {} for reading, {}",
                  fortune_db_path.string(),
                  mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    auto strfile_header_buffer = array<char, sizeof(StrFileHeader)>{};
    fortune_db_file.read(data(strfile_header_buffer), size(strfile_header_buffer));
    if (fortune_db_file.fail() || fortune_db_file.bad()) {
        LOG_ERROR("unable to read STRFILE database header {}, {}",
                  fortune_db_path.string(),
                  mmotd::error::ios_flags::to_string(fortune_db_file));
        return nullopt;
    }

    auto strfile_header = StrFileHeader{};
    memcpy(&strfile_header, data(strfile_header_buffer), size(strfile_header_buffer));
    strfile_header.update();

    if (strfile_header.version != STRFILE_VERSION) {
        LOG_ERROR("STRFILE database header is version {} not the expected version {}",
                  strfile_header.version,
                  STRFILE_VERSION);
        return nullopt;
    }

    LOG_VERBOSE("sizeof STRFILE header: {}", sizeof(StrFileHeader) + STRFILE_HEADER_PADDING);
    auto remaining_size = fortune_db_file_size - sizeof(StrFileHeader) - STRFILE_ENTRY_SIZE;
    LOG_VERBOSE("remaining size : {}", remaining_size);
    LOG_VERBOSE("number of strs : {}", strfile_header.count);
    LOG_VERBOSE("calculated strs: {} with {} bytes remaining",
                remaining_size / STRFILE_ENTRY_SIZE,
                remaining_size % STRFILE_ENTRY_SIZE);

    auto random_index = effing_random::get<size_t>(0, static_cast<size_t>(strfile_header.count));
    LOG_VERBOSE("random STRFILE database index: {}", random_index);
    auto file_offset = ConvertDbIndexToFortuneFileOffset(random_index, static_cast<uint32_t>(fortune_db_file_size));

    auto fortune_offset = ReadRandomFortuneOffset(fortune_db_path, fortune_db_file, file_offset);
    if (!fortune_offset) {
        return nullopt;
    }

    return make_optional(make_tuple(*fortune_offset, strfile_header.longest_str, strfile_header.get_delim()));
}

optional<string> ReadFortune(const fs::path &fortune_path, uint32_t offset, uint32_t max_size, char delimeter) {
    auto fortune_file = ifstream{};
    fortune_file.exceptions(std::ifstream::goodbit);
    fortune_file.open(fortune_path, ios_base::in);

    if (!fortune_file.is_open() || fortune_file.fail() || fortune_file.bad()) {
        LOG_ERROR("unable to open {} for reading, {}",
                  fortune_path.string(),
                  mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    fortune_file.seekg(offset);
    if (!fortune_file.good()) {
        LOG_ERROR("unable to seek {} to fortune at offset {}, {}",
                  fortune_path.string(),
                  offset,
                  mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    auto buffer = vector<char>(max_size, 0);
    fortune_file.read(buffer.data(), static_cast<streamsize>(size(buffer)));
    if (fortune_file.fail() || fortune_file.bad()) {
        LOG_ERROR("unable to read {} bytes of the fortune file {} at offset {}, {}",
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
    LOG_VERBOSE("fortune:\n{}", fortune);
    return make_optional(trim_right_copy(fortune));
}

optional<string> GetRandomFortune(string fortune_filename, string fortune_db_dir) {
    auto fortune_files_holder = GetFortuneFiles(fortune_filename, fortune_db_dir);
    if (!fortune_files_holder) {
        return nullopt;
    }

    auto [fortune_path, fortune_db_path] = *fortune_files_holder;

    auto fortune_offset_holder = GetRandomFortuneOffset(fortune_db_path);
    if (!fortune_offset_holder) {
        return nullopt;
    }
    auto [fortune_offset, max_fortune_size, fortune_delimeter] = *fortune_offset_holder;

    auto ec = std::error_code{};
    auto fortune_file_size = fs::file_size(fortune_path, ec);
    if (ec) {
        LOG_ERROR("error while getting file size of {}, details: {}", fortune_path.string(), ec.message());
        return nullopt;
    }

    if (fortune_offset >= fortune_file_size) {
        LOG_ERROR("the fortune offset={} is beyond the fortune file size={}", fortune_offset, fortune_file_size);
        return nullopt;
    } else if (fortune_offset + max_fortune_size > fortune_file_size) {
        LOG_DEBUG("updating fortune read size (offset={} + read size={}) since it is now past eof (file size={} bytes)",
                  fortune_offset,
                  max_fortune_size,
                  fortune_file_size);
        max_fortune_size = static_cast<uint32_t>(fortune_file_size) - fortune_offset;
        LOG_DEBUG("updated fortune read size (offset={} + read size={}) should now be to the eof (file size={} bytes)",
                  fortune_offset,
                  max_fortune_size,
                  fortune_file_size);
    }

    return ReadFortune(fortune_path, fortune_offset, max_fortune_size, fortune_delimeter);
}

struct TextFortune {
    vector<string> text;
    void append(string &&new_text) { text.emplace_back(new_text); }
    void append(const string &new_text) { text.push_back(new_text); }
    string to_string() const { return mmotd::algorithms::join(text, "\n"); }
};

using TextFortunes = vector<TextFortune>;

optional<string> ReadTextFromFortunesFile(fs::path fortune_path) {
    auto fortune_file = ifstream{};
    fortune_file.exceptions(std::ifstream::goodbit);
    fortune_file.open(fortune_path, ios_base::in);

    if (!fortune_file.is_open() || fortune_file.fail() || fortune_file.bad()) {
        LOG_ERROR("unable to open {} for reading, {}",
                  fortune_path.string(),
                  mmotd::error::ios_flags::to_string(fortune_file));
        return nullopt;
    }

    LOG_DEBUG("found text fortunes and opened it for reading: {}", fortune_path.string());
    return make_optional(string(istreambuf_iterator<char>(fortune_file), istreambuf_iterator<char>()));
}

TextFortunes ParseTextFortunes(string fortune_text) {
    auto fortune_strm = istringstream{fortune_text};
    auto text_fortunes = TextFortunes{1ull, TextFortune{}};
    for (std::string line; std::getline(fortune_strm, line);) {
        if (boost::trim_copy(line) == "%") {
            text_fortunes.emplace_back(TextFortune{});
        } else {
            text_fortunes.back().append(boost::trim_right_copy(line));
        }
    }
    return text_fortunes;
}

optional<string> ReadTextFortuneFile(fs::path fortune_path) {
    auto text = ReadTextFromFortunesFile(fortune_path);
    if (!text) {
        LOG_ERROR("no text was found within: {}", quoted(fortune_path.string()));
        return nullopt;
    }
    auto text_fortunes = ParseTextFortunes(*text);
    if (empty(text_fortunes)) {
        LOG_ERROR("no text fortunes were parsed from: {}", quoted(fortune_path.string()));
        return nullopt;
    }
    auto iter = effing_random::get(text_fortunes);
    if (iter == end(text_fortunes)) {
        LOG_ERROR("unable to select a random fortune from: {}", quoted(fortune_path.string()));
        return nullopt;
    }
    LOG_DEBUG("choose random fortune {} from {} total fortunes in: ",
              std::distance(begin(text_fortunes), iter),
              size(text_fortunes),
              fortune_path.string());
    const auto &fortune = *iter;
    const auto &fortune_str = fortune.to_string();
    LOG_DEBUG("random fortune:\n{}", fortune_str);
    return fortune_str;
}

optional<string> GetRandomFortune(string fortune_filename) {
    using namespace mmotd::core::special_files;
    auto fortune_path = fs::path{fortune_filename}.replace_extension(".txt");
    fortune_path = FindFileInDefaultLocations(fortune_path);
    if (empty(fortune_path)) {
        LOG_ERROR("unable to find a text fortune file: {}", fortune_filename);
        return nullopt;
    }
    LOG_DEBUG("found text fortunes path: {}", fortune_path.string());
    return ReadTextFortuneFile(fortune_path);
}

} // namespace

namespace mmotd::information {

void Fortune::FindInformation() {
    using namespace mmotd::core;
    auto fortune_filename = ConfigOptions::Instance().GetString("fortune.file_name"sv, "softwareengineering"sv);
    auto fortune_db_dir = ConfigOptions::Instance().GetString("fortune.db_directory"sv, GetPlatformFortunesPath());

    if (auto fortune_holder1 = GetRandomFortune(fortune_filename, fortune_db_dir); fortune_holder1) {
        AddFortune(std::move(*fortune_holder1));
    } else if (auto fortune_holder2 = GetRandomFortune(fortune_filename); fortune_holder2) {
        AddFortune(std::move(*fortune_holder2));
    } else {
        LOG_ERROR("unable to find a fortune");
    }
}

void Fortune::AddFortune(const std::string &fortune_str) {
    if (empty(fortune_str)) {
        LOG_ERROR("unable to add an empty fortune");
        return;
    }
    auto fortune = GetInfoTemplate(InformationId::ID_FORTUNE_FORTUNE);
    fortune.SetValueArgs(fortune_str);
    AddInformation(fortune);
}

} // namespace mmotd::information
