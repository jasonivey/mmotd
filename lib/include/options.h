// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

#include <boost/any.hpp>
#include <optional>
#include <string>
#include <vector>

struct Option {
    Option() = default;
    Option(const std::string &, const boost::any &);

    std::string name;
    boost::any value;
};

using Options = std::vector<Option>;

class AppOptions {
public:
    static std::optional<AppOptions> Initialize(int argc, char **argv);
    const Options &GetOptions() const { return options_; }

private:
    AppOptions() = default;
    static AppOptions &GetAppOptions();
    bool ParseCommandLine(int argc, char **argv);

    Options options_;
};
