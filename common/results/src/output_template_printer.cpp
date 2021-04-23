// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/results/include/output_frame.h"
#include "common/results/include/output_template_printer.h"

#include <fmt/format.h>

namespace mmotd::results {

void PrintOutputTemplate(const OutputTemplate &output_template, const mmotd::information::Informations &informations) {
    auto frame = Frame::CreateFrame(output_template, informations);
    fmt::print(FMT_STRING("{}\n"), frame.to_string());
}

} // namespace mmotd::results
