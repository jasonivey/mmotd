// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

//class Component : std::enable_shared_from_this<Component> {
//    friend std::shared_ptr<Component> RegisterComponent(std::string_view string_view);
class Component {

public:
    Component() = default;
    Component(std::string_view name, std::string_view value);
    virtual ~Component() = default;

private:
    //using Components = std::unordered_map<std::string, Component>;

    //Components components_;
};

