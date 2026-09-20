#include "log.h"
#include "config.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

namespace {

bool isOk(const char* tag) {
    return _stricmp(tag, "OK") == 0;
}

void logTagged(const char* color, const char* tag, const char* message) {
    size_t pad = strlen(tag) < 6 ? 6 - strlen(tag) : 1;
    printf(WHITE "[" RESET "%s%s" RESET WHITE "]" RESET "%*s%s\n",
           color, tag, (int)pad, "", message);
}

}

void logLine(const char* tag, const char* message) {
    logTagged(isOk(tag) ? GREEN : RED, tag, message);
}

void logAction(const char* tag, const char* message) {
    logTagged(isOk(tag) ? GREEN : RED, tag, message);
}

void logInfo(const char* message) {
    printf(WHITE "%s" RESET "\n", message);
}

void logDebug(const char* message) {
    if (!cfg::debugMode) return;
    logTagged(GRAY, "DBG", message);
}