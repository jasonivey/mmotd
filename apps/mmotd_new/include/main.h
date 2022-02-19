// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

bool ForceLinkOptionalProviders() {
    // mmotd-lib components
#if !defined(OMIT_LINK_EXTERNAL_NETWORK_INFO)
    extern bool gLinkExternalNetwork;
    gLinkExternalNetwork = true;
#endif
#if !defined(OMIT_LINK_LOAD_AVERAGE_INFO)
    extern bool gLinkLoadAverage;
    gLinkLoadAverage = true;
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
    extern bool gLinkSystemInformation;
    gLinkSystemInformation = true;
#endif
#if !defined(OMIT_LINK_FILE_SYSTEM)
    extern bool gLinkFileSystem;
    gLinkFileSystem = true;
#endif
#if !defined(OMIT_LINK_MEMORY_USAGE)
    extern bool gLinkMemoryUsage;
    gLinkMemoryUsage = true;
#endif
#if !defined(OMIT_LINK_USERS_LOGGED_IN)
    extern bool gLinkUsersLoggedIn;
    gLinkUsersLoggedIn = true;
#endif
#if !defined(OMIT_LINK_SWAP_USAGE)
    extern bool gLinkSwapUsage;
    gLinkSwapUsage = true;
#endif
#if !defined(OMIT_LINK_PROCESSES_INFO)
    extern bool gLinkProcessesInfo;
    gLinkProcessesInfo = true;
#endif
#if !defined(OMIT_LINK_FORTUNE_GENERATOR)
    extern bool gLinkFortuneGenerator;
    gLinkFortuneGenerator = true;
#endif
#if !defined(OMIT_LINK_BOOT_TIME)
    extern bool gLinkBootTime;
    gLinkBootTime = true;
#endif
#if !defined(OMIT_LINK_WEATHER_INFO)
    extern bool gLinkWeatherInfo;
    gLinkWeatherInfo = true;
#endif
#if !defined(OMIT_LINK_GENERAL_INFO)
    extern bool gLinkGeneralGenerator;
    gLinkGeneralGenerator = true;
#endif
#if !defined(OMIT_LINK_PACKAGE_MANAGEMENT)
    extern bool gLinkPackageManagementInfo;
    gLinkPackageManagementInfo = true;
#endif
#if !defined(OMIT_LINK_HARDWARE_INFORMATION)
    extern bool gLinkHardwareInformation;
    gLinkHardwareInformation = true;
#endif
    return true;
}

static bool forced_linking_enabled = ForceLinkOptionalProviders();
