// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/assertion/include/assertion.h"
#include "common/include/mac_address.h"

#include <cstring>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <fmt/format.h>
#include <plog/Log.h>

using namespace std;
using boost::numeric_cast;
using fmt::format;

namespace mmotd::networking {

MacAddress::MacAddress(const uint8_t *buffer, size_t buffer_size) {
    if (buffer_size != MAC_ADDRESS_SIZE) {
        MMOTD_THROW_INVALID_ARGUMENT("invalid size of mac address");
    } else if (buffer != nullptr) {
        memcpy(&data_[0], buffer, MAC_ADDRESS_SIZE);
    } else {
        memset(&data_[0], 0, MAC_ADDRESS_SIZE);
    }
}

// converts the following strings to a MacAddress:
// '64:4b:f0:27:6a:76', '64.4b.f0.27.6a.76', '64 4b f0 27 6a 76' or '644bf0276a76'
MacAddress MacAddress::from_string(const std::string &input_str) {
    using namespace boost::algorithm;

    auto mac_addr_hex_chars = vector<string>{};
    split(mac_addr_hex_chars, input_str, is_any_of(" :."), token_compress_on);

    // if it didn't split at all -- the 6 double digit hex chars must not be delimitted by anything
    if (mac_addr_hex_chars.size() == 1) {
        mac_addr_hex_chars.clear();
        auto hex_char = string{};
        for_each(begin(input_str), end(input_str), [&mac_addr_hex_chars, &hex_char](const auto &single_char) {
            if (!hex_char.empty()) {
                mac_addr_hex_chars.push_back(hex_char + single_char);
                hex_char.clear();
            } else {
                hex_char.push_back(single_char);
            }
        });
    }

    // if the number of elements are still not == 6 then we have malformed input
    if (mac_addr_hex_chars.size() != MAC_ADDRESS_SIZE) {
        auto error_str =
            format(FMT_STRING("mac address is not the correct length (size={} != 6)"), mac_addr_hex_chars.size());
        MMOTD_THROW_INVALID_ARGUMENT(error_str);
    }

    // convert each two character element into an unsigned long integer and store it in the raw mac address
    auto raw_mac_addr = vector<uint8_t>(MAC_ADDRESS_SIZE, 0);
    transform(begin(mac_addr_hex_chars),
              end(mac_addr_hex_chars),
              begin(raw_mac_addr),
              [&input_str](const auto &hex_char) {
                  if (!all(hex_char, is_xdigit()) || hex_char.empty() || hex_char.size() > 2) {
                      auto error_str =
                          format(FMT_STRING("invalid character (\"{}\") found in mac address {}"), hex_char, input_str);
                      MMOTD_THROW_INVALID_ARGUMENT(error_str);
                  }
                  auto value = std::stoul(hex_char, nullptr, 16);
                  return numeric_cast<uint8_t>(value);
              });

    // finally create a MacAddress instance
    return MacAddress{raw_mac_addr.data(), MAC_ADDRESS_SIZE};
}

bool MacAddress::IsZero() const {
    static constexpr array<uint8_t, MAC_ADDRESS_SIZE> EMPTY_MAC_ADDRESS = {0, 0, 0, 0, 0, 0};
    return memcmp(data_.data(), &EMPTY_MAC_ADDRESS, MAC_ADDRESS_SIZE) == 0;
}

string MacAddress::to_string() const {
    return format(FMT_STRING("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}"),
                  static_cast<uint32_t>(data_[0]),
                  static_cast<uint32_t>(data_[1]),
                  static_cast<uint32_t>(data_[2]),
                  static_cast<uint32_t>(data_[3]),
                  static_cast<uint32_t>(data_[4]),
                  static_cast<uint32_t>(data_[5]));
}

} // namespace mmotd::networking
