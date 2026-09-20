#pragma once

#include <string>

namespace inject {
    enum class Result { OK, TARGET_NOT_FOUND, FAILED };

    Result inject(unsigned long processId, const std::string& dllPath, const wchar_t* targetName);
    bool findPID(const wchar_t* name, unsigned long& processId);
}