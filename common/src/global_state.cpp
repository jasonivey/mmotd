// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/global_state.h"

#include <curl/curl.h>

namespace mmotd::globals {

GlobalState::GlobalState() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

GlobalState::~GlobalState() {
    curl_global_cleanup();
}

} // namespace mmotd::globals
