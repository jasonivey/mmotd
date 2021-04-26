// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once
#include <string>

namespace mmotd::source_location {

enum class FunctionArgStrategy { remove, replace, none };
enum class FunctionReturnStrategy { remove, none };

std::string TrimFileName(const char *file) noexcept;
std::string TrimFunction(const char *function,
                         FunctionArgStrategy arg_strategy = FunctionArgStrategy::remove,
                         FunctionReturnStrategy return_strategy = FunctionReturnStrategy::none) noexcept;

std::string StripFunctionArgs(std::string function) noexcept;
std::string StripFunctionReturn(std::string function) noexcept;
std::string StripAnonymousNamespace(std::string function) noexcept;
std::string StripNamespaces(std::string function) noexcept;

} // namespace mmotd::source_location
