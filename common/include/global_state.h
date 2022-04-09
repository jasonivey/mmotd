// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

namespace mmotd::globals {

class GlobalState {
public:
    GlobalState();
    ~GlobalState();
    GlobalState(GlobalState const &other) = delete;
    GlobalState &operator=(GlobalState const &other) = delete;
    GlobalState(GlobalState &&other) = delete;
    GlobalState &operator=(GlobalState &&other) = delete;
};

} // namespace mmotd::globals
