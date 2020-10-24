// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>

namespace Logging {

void DefaultInitializeLogging();
void InitializeLogging(const std::string &ini_file);
void UpdateSeverityFilter(int severity);

}

