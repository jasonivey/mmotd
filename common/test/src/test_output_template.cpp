#include "common/results/include/output_template.h"
#include "common/results/include/template_column_items.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

using nlohmann::json;
namespace fs = std::filesystem;
using Catch::Matchers::Equals;
using namespace std;

namespace {

fs::path FindOneColumnOutputTemplateFileName() {
    error_code ec;
    auto current_dir = fs::absolute(fs::current_path(ec));
    CHECK(!ec);
    current_dir = fs::absolute(current_dir, ec);
    CHECK(!ec);

    static const string ONE_COLUMN_OUTPUT_TEMPLATE_RELATIVE_PATH = {"config/mmotd_1_column_template.json"};

    auto result = fs::path{};
    while (result.empty() && current_dir != current_dir.root_directory()) {
        auto one_column_output_template_path = current_dir / ONE_COLUMN_OUTPUT_TEMPLATE_RELATIVE_PATH;
        if (fs::is_regular_file(one_column_output_template_path, ec) && !ec) {
            result = fs::absolute(one_column_output_template_path, ec);
            CHECK(!ec);
        } else {
            current_dir = fs::absolute(current_dir / "..", ec);
            CHECK(!ec);
        }
    }
    return result;
}

} // namespace

namespace mmotd::results::test {

TEST_CASE("default template matches 1 column template", "[OutputTemplate]") {
    auto one_column_output_template_path = FindOneColumnOutputTemplateFileName();
    CHECK(!one_column_output_template_path.empty());

    auto output_template = OutputTemplate{};
    auto default_json_output_template = json();

    output_template.to_json(default_json_output_template);

    auto input = ifstream(one_column_output_template_path.string());
    CHECK(input.is_open());

    auto one_column_json_output_template = json();
    one_column_json_output_template = json::parse(input);

    CHECK(one_column_json_output_template == default_json_output_template);
}

} // namespace mmotd::results::test
