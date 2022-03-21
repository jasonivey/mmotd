// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/output_template.h"
#include "common/include/special_files.h"
#include "common/include/template_column_items.h"

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

fs::path FindDefaultTemplatePath() {
    auto project_root = mmotd::core::special_files::FindProjectRootDirectory();
    if (empty(project_root)) {
        return fs::path{};
    }
    auto ec = error_code{};
    auto template_path = project_root / "config" / "mmotd_template.json";
    if (fs::exists(template_path, ec) && !ec) {
        return template_path;
    } else {
        return fs::path{};
    }
}

} // namespace

namespace mmotd::output_template::test {

CATCH_TEST_CASE("default output template matches mmotd_template.json", "[OutputTemplate]") {
    auto default_template_path = FindDefaultTemplatePath();
    CATCH_CHECK(!default_template_path.empty());

    auto default_template_stream = ifstream(default_template_path);
    CATCH_CHECK(default_template_stream.is_open());
    auto default_template_json = json::parse(default_template_stream);

    auto output_template = OutputTemplate{};
    auto default_output_template_json = json();
    output_template.to_json(default_output_template_json);

    CATCH_CHECK(default_template_json == default_output_template_json);
}

} // namespace mmotd::output_template::test
