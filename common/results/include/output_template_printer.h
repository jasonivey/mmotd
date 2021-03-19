// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <vector>

namespace mmotd::information {

class Informations;

} // namespace mmotd::information

namespace mmotd::results {

class OutputTemplate;

void PrintOutputTemplate(const OutputTemplate &output_template, const mmotd::information::Informations &informations);

} // namespace mmotd::results
