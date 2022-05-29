// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "common/include/global_state.h"

#include "common/assertion/include/assertion.h"

#include <curl/curl.h>

namespace mmotd::globals {

GlobalState::GlobalState() {
    auto ret_code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (ret_code != CURLE_OK) {
        THROW_RUNTIME_ERROR("curl_global_init failed, {}: {}", ret_code, curl_easy_strerror(ret_code));
    }
}

GlobalState::~GlobalState() {
    curl_global_cleanup();
}

} // namespace mmotd::globals
