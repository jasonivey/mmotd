// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/algorithm.h"
#include "common/include/human_size.h"

#include <stdexcept>
#include <string_view>

#include <catch2/catch.hpp>
#include <fmt/format.h>

using namespace Catch;
using namespace Catch::Matchers;
using namespace std;
using namespace mmotd::algorithms;

namespace mmotd::test {

CATCH_TEST_CASE("split_sentence", "[algorithms]") {
    CATCH_SECTION("split-empty-sentence") {
        CATCH_CHECK(split_sentence(string{}, 80ull) == string{});
        CATCH_CHECK(split_sentence(string{}, 0ull) == string{});
    }
    CATCH_SECTION("ensure-split-sentence-size") {
        static const auto sentence = string{"some text here"};
        CATCH_CHECK(split_sentence(sentence, size(sentence) - 1) == string{"some text\nhere"});
        CATCH_CHECK(split_sentence(sentence, size(sentence)) == sentence);
        CATCH_CHECK(split_sentence(sentence, size(sentence) + 1) == sentence);
    }
    CATCH_SECTION("large-sentence-split-40") {
        const auto fixed_input = string{
            R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc bibendum lacinia orci. Nullam a pretium nulla, sagittis ultrices est. Curabitur ultricies eget dui vel faucibus. Etiam id lacus purus. Suspendisse maximus augue massa, sagittis consectetur felis viverra non. Phasellus ac purus aliquam lacus venenatis aliquet quis tristique sapien. Nam eget rutrum orci. Maecenas viverra odio mi, sit amet cursus magna convallis vitae. Donec lectus justo, iaculis et justo in, consectetur malesuada sem. Nunc consectetur sed massa et sodales. Suspendisse elementum in risus at euismod. Donec posuere tristique lectus, ut vulputate nibh condimentum sed. Morbi ac orci in diam luctus pharetra at nec justo.)"};
        const auto fixed_output = string{R"(Lorem ipsum dolor sit amet, consectetur
adipiscing elit. Nunc bibendum lacinia
orci. Nullam a pretium nulla, sagittis
ultrices est. Curabitur ultricies eget
dui vel faucibus. Etiam id lacus purus.
Suspendisse maximus augue massa,
sagittis consectetur felis viverra non.
Phasellus ac purus aliquam lacus
venenatis aliquet quis tristique
sapien. Nam eget rutrum orci. Maecenas
viverra odio mi, sit amet cursus magna
convallis vitae. Donec lectus justo,
iaculis et justo in, consectetur
malesuada sem. Nunc consectetur sed
massa et sodales. Suspendisse elementum
in risus at euismod. Donec posuere
tristique lectus, ut vulputate nibh
condimentum sed. Morbi ac orci in diam
luctus pharetra at nec justo.)"};
        auto output = split_sentence(fixed_input, 40);
        CATCH_CHECK(output == fixed_output);
    }
    CATCH_SECTION("large-sentence-split-80") {
        const auto fixed_input = string{
            R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc bibendum lacinia orci. Nullam a pretium nulla, sagittis ultrices est. Curabitur ultricies eget dui vel faucibus. Etiam id lacus purus. Suspendisse maximus augue massa, sagittis consectetur felis viverra non. Phasellus ac purus aliquam lacus venenatis aliquet quis tristique sapien. Nam eget rutrum orci. Maecenas viverra odio mi, sit amet cursus magna convallis vitae. Donec lectus justo, iaculis et justo in, consectetur malesuada sem. Nunc consectetur sed massa et sodales. Suspendisse elementum in risus at euismod. Donec posuere tristique lectus, ut vulputate nibh condimentum sed. Morbi ac orci in diam luctus pharetra at nec justo.)"};
        const auto fixed_output =
            string{R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc bibendum lacinia
orci. Nullam a pretium nulla, sagittis ultrices est. Curabitur ultricies eget
dui vel faucibus. Etiam id lacus purus. Suspendisse maximus augue massa,
sagittis consectetur felis viverra non. Phasellus ac purus aliquam lacus
venenatis aliquet quis tristique sapien. Nam eget rutrum orci. Maecenas viverra
odio mi, sit amet cursus magna convallis vitae. Donec lectus justo, iaculis et
justo in, consectetur malesuada sem. Nunc consectetur sed massa et sodales.
Suspendisse elementum in risus at euismod. Donec posuere tristique lectus, ut
vulputate nibh condimentum sed. Morbi ac orci in diam luctus pharetra at nec
justo.)"};
        auto output = split_sentence(fixed_input, 80);
        CATCH_CHECK(output == fixed_output);
    }
    CATCH_SECTION("large-word-sentence-split") {
        const auto fixed_input = string{"pneumonoultramicroscopicsilicovolcanoconiosis"};
        CATCH_CHECK(split_sentence(fixed_input, 30) == fixed_input);
        CATCH_CHECK(split_sentence(fixed_input, 40) == fixed_input);
        CATCH_CHECK(split_sentence(fixed_input, 50) == fixed_input);
    }
    CATCH_SECTION("large-word-start-sentence-split") {
        const auto fixed_input = string{
            R"(Pneumonoultramicroscopicsilicovolcanoconiosis is lung disease caused by the inhalation of silica or quartz dust.)"};
        const auto fixed_output = string{
            R"(Pneumonoultramicroscopicsilicovolcanoconiosis
is lung disease caused by the
inhalation of silica or quartz dust.)"};
        auto output = split_sentence(fixed_input, 40);
        CATCH_CHECK(output == fixed_output);
    }
    CATCH_SECTION("large-word-middle-sentence-split") {
        const auto fixed_input = string{
            R"(A lung disease known as pneumonoultramicroscopicsilicovolcanoconiosis is caused by the inhalation of silica or quartz dust.)"};
        const auto fixed_output = string{
            R"(A lung disease known as
pneumonoultramicroscopicsilicovolcanoconiosis
is caused by the inhalation of silica
or quartz dust.)"};
        auto output = split_sentence(fixed_input, 40);
        CATCH_CHECK(output == fixed_output);
    }
    CATCH_SECTION("large-word-end-sentence-split") {
        const auto fixed_input = string{
            R"(A lung disease caused by the inhalation of silica or quartz dust is called pneumonoultramicroscopicsilicovolcanoconiosis.)"};
        const auto fixed_output = string{
            R"(A lung disease caused by the inhalation
of silica or quartz dust is called
pneumonoultramicroscopicsilicovolcanoconiosis.)"};
        auto output = split_sentence(fixed_input, 40);
        CATCH_CHECK(output == fixed_output);
    }
}

CATCH_TEST_CASE("to_human_size", "[algorithms]") {
    using mmotd::algorithm::string::to_human_size;

    CATCH_SECTION("integers") {
        CATCH_CHECK(to_human_size(0) == "0.00 B");
        CATCH_CHECK(to_human_size(1) == "1.00 B");
        CATCH_CHECK(to_human_size(10) == "10.00 B");
        CATCH_CHECK(to_human_size(100) == "100.00 B");
        CATCH_CHECK(to_human_size(1000) == "1000.00 B");
        CATCH_CHECK(to_human_size(1024) == "1.00 KB");
        CATCH_CHECK(to_human_size(1000000) == "976.56 KB");
        CATCH_CHECK(to_human_size(1024 * 1024) == "1.00 MB");
        CATCH_CHECK(to_human_size(1000000000) == "953.67 MB");
        CATCH_CHECK(to_human_size(1024ull * 1024ull * 1024ull) == "1.00 GB");
        CATCH_CHECK(to_human_size(1000000000000ull) == "931.32 GB");
        CATCH_CHECK(to_human_size(1024ull * 1024ull * 1024ull * 1024ull) == "1.00 TB");
        CATCH_CHECK(to_human_size(1000000000000000ull) == "909.49 TB");
        CATCH_CHECK(to_human_size(1024ull * 1024ull * 1024ull * 1024ull * 1024ull) == "1.00 PB");
        CATCH_CHECK(to_human_size(1000000000000000000ull) == "888.18 PB");
        CATCH_CHECK(to_human_size(1024ull * 1024ull * 1024ull * 1024ull * 1024ull * 1024ull) == "1.00 EB");
    }

    CATCH_SECTION("floating-point") {
        CATCH_CHECK(to_human_size(0.0) == "0.00 B");
        CATCH_CHECK(to_human_size(1.0) == "1.00 B");
        CATCH_CHECK(to_human_size(10.0) == "10.00 B");
        CATCH_CHECK(to_human_size(100.0) == "100.00 B");
        CATCH_CHECK(to_human_size(1000.0) == "1000.00 B");
        CATCH_CHECK(to_human_size(1024.0) == "1.00 KB");
        CATCH_CHECK(to_human_size(1000000.0) == "976.56 KB");
        CATCH_CHECK(to_human_size(1024.0 * 1024.0) == "1.00 MB");
        CATCH_CHECK(to_human_size(1000000000.0) == "953.67 MB");
        CATCH_CHECK(to_human_size(1024.0 * 1024.0 * 1024.0) == "1.00 GB");
        CATCH_CHECK(to_human_size(1000000000000.0) == "931.32 GB");
        CATCH_CHECK(to_human_size(1024.0 * 1024.0 * 1024.0 * 1024.0) == "1.00 TB");
        CATCH_CHECK(to_human_size(1000000000000000.0) == "909.49 TB");
        CATCH_CHECK(to_human_size(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0) == "1.00 PB");
        CATCH_CHECK(to_human_size(1000000000000000000.0) == "888.18 PB");
        CATCH_CHECK(to_human_size(1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0) == "1.00 EB");
    }
}

} // namespace mmotd::test
