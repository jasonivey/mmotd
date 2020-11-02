// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <string>
#if defined(_PERFORMANCE_)
#include <utility>
#include <vector>
using Component = std::pair<std::string, std::string>;
using Componenets = std::vector<Component>;
#else
#include <unordered_map>
using Componenets = std::unordered_map<std::string, std::string>;
#endif

class IModel;

class IMediator {
public:
    virtual ~IMediator() = default;

    virtual const Componenets &GetAllComponenets() const = 0;
    virtual void LoadAllComponents() = 0;
};
