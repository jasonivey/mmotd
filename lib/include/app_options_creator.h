// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

struct Options;

class AppOptionsCreator {
public:
    AppOptionsCreator();

    virtual ~AppOptionsCreator();

    virtual const Options &GetOptions() const = 0;

    virtual bool IsAppFinished() const = 0;

private:
    //virtual void AddOption(const Option &option) = 0;
};
