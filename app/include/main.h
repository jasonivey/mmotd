// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

bool ForceLinkOptionalProviders() {
    // mmotd-view components
#if !defined(OMIT_LINK_ACTIVE_NETWORK_INTERFACES_PROVIDER)
    extern bool gLinkActiveNetworkInterfaces;
    gLinkActiveNetworkInterfaces = true;
#endif
#if !defined(OMIT_LINK_COMPUTER_NAME_PROVIDER)
    extern bool gLinkComputerNameProvider;
    gLinkComputerNameProvider = true;
#endif
#if !defined(OMIT_LINK_DISK_USAGE_PROVIDER)
    extern bool gLinkDiskUsageProvider;
    gLinkDiskUsageProvider = true;
#endif
#if !defined(OMIT_LINK_GREETING_PROVIDER)
    extern bool gLinkGreetingProvider;
    gLinkGreetingProvider = true;
#endif
#if !defined(OMIT_LINK_HEADER_PROVIDER)
    extern bool gLinkHeaderProvider;
    gLinkHeaderProvider = true;
#endif
#if !defined(OMIT_LINK_HOST_NAME_PROVIDER)
    extern bool gLinkHostNameProvider;
    gLinkHostNameProvider = true;
#endif
#if !defined(OMIT_LINK_LAST_LOGIN_PROVIDER)
    extern bool gLinkLastLoginProvider;
    gLinkLastLoginProvider = true;
#endif
#if !defined(OMIT_LINK_MEMORY_USAGE_PROVIDER)
    extern bool gLinkMemoryUsageProvider;
    gLinkMemoryUsageProvider = true;
#endif
#if !defined(OMIT_LINK_PROCESSOR_COUNT_PROVIDER)
    extern bool gLinkProcessorCountProvider;
    gLinkProcessorCountProvider = true;
#endif
#if !defined(OMIT_LINK_PUBLIC_IP_PROVIDER)
    extern bool gLinkPublicIpProvider;
    gLinkPublicIpProvider = true;
#endif
#if !defined(OMIT_LINK_RANDOM_QUOTE_PROVIDER)
    extern bool gLinkRandomQuoteProvider;
    gLinkRandomQuoteProvider = true;
#endif
#if !defined(OMIT_LINK_SUBHEADER_PROVIDER)
    extern bool gLinkSubHeaderProvider;
    gLinkSubHeaderProvider = true;
#endif
#if !defined(OMIT_LINK_SWAP_USAGE_PROVIDER)
    extern bool gLinkSwapUsageProvider;
    gLinkSwapUsageProvider = true;
#endif
#if !defined(OMIT_LINK_SYSTEM_LOAD_PROVIDER)
    extern bool gLinkSystemLoadProvider;
    gLinkSystemLoadProvider = true;
#endif
#if !defined(OMIT_LINK_UNREAD_MAIL_PROVIDER)
    extern bool gLinkUnreadMailProvider;
    gLinkUnreadMailProvider = true;
#endif
#if !defined(OMIT_LINK_USERS_COUNT_PROVIDER)
    extern bool gLinkUsersCountProvider;
    gLinkUsersCountProvider = true;
#endif

    // mmotd-lib components
#if !defined(OMIT_LINK_EXTERNAL_NETWORK_INFO)
    extern bool gLinkExternalNetwork;
    gLinkExternalNetwork = true;
#endif
#if !defined(OMIT_LINK_LAST_LOG_INFO)
    extern bool gLinkLastLog;
    gLinkLastLog = true;
#endif
#if !defined(OMIT_LINK_NETWORK_INFO)
    extern bool gLinkNetworkInformation;
    gLinkNetworkInformation = true;
#endif
#if !defined(OMIT_LINK_POSIX_SYSTEM_INFO)
    extern bool gLinkPosixSystemInformation;
    gLinkPosixSystemInformation = true;
#endif

    return true;
}

static bool forced_linking_enabled = ForceLinkOptionalProviders();
