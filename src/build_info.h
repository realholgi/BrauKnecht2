#pragma once

#include <stddef.h>
#include <stdio.h>

#if __has_include("BuildInfoGenerated.h")
#include "BuildInfoGenerated.h"
#endif

#ifndef VERSION_STRING
#define VERSION_STRING "0.0.0"
#endif

#ifndef BUILD_GIT_HASH
#define BUILD_GIT_HASH "unknown"
#endif

#ifndef BUILD_TIME
#define BUILD_TIME "unknown"
#endif

#ifndef BUILD_ENV
#define BUILD_ENV "unknown"
#endif

inline const char *firmwareVersion() {
    return VERSION_STRING;
}

inline const char *buildGitHash() {
    return BUILD_GIT_HASH;
}

inline const char *buildTime() {
    return BUILD_TIME;
}

inline const char *buildEnvironment() {
    return BUILD_ENV;
}

inline void formatBuildTime(char *out, size_t n, const char *isoTime) {
    if (n == 0) {
        return;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    if (isoTime && sscanf(isoTime, "%4d-%2d-%2dT%2d:%2d", &year, &month, &day, &hour, &minute) == 5) {
        snprintf(out, n, "%02d.%02d.%04d %02d:%02d UTC", day, month, year, hour, minute);
        return;
    }

    snprintf(out, n, "%s", isoTime && isoTime[0] ? isoTime : "unknown");
}
