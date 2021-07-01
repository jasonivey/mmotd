// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace mmotd::networking {

struct MacAddress {
    MacAddress();
    MacAddress(const std::uint8_t *buffer, std::size_t buffer_size);

    explicit operator bool() const { return !IsZero(); }
    std::string to_string() const;
    static MacAddress from_string(const std::string &input_str);

    uint8_t *data() { return data_.data(); }
    const uint8_t *data() const { return data_.data(); }

private:
    bool IsZero() const;

    static constexpr size_t MAC_ADDRESS_SIZE = 6;
    std::array<std::uint8_t, MAC_ADDRESS_SIZE> data_ = {0, 0, 0, 0, 0, 0};
};

} // namespace mmotd::networking
