// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace mmotd {

class ComputerInformationProvider;
using ComputerInformationProviderPtr = std::unique_ptr<ComputerInformationProvider>;
using ComputerInformationProviderCreator = std::function<ComputerInformationProviderPtr()>;

bool RegisterComputerInformationProvider(ComputerInformationProviderCreator &&creator);

using ComputerInformationProviders = std::vector<ComputerInformationProviderPtr>;
ComputerInformationProviders GetComputerInformationProviders();

} // namespace mmotd
