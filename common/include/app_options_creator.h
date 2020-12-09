// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

struct Options;

class AppOptionsCreator {
public:
    AppOptionsCreator() = default;
    virtual ~AppOptionsCreator() = default;

    virtual const Options &GetOptions() const = 0;
    virtual bool IsAppFinished() const = 0;

private:
};
