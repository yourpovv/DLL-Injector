#pragma once

namespace cfg {
    // process name to wait for, leave empty to choose on launch
    inline constexpr const wchar_t* targetProcess = L""; // e.g., L"processName.exe";

    inline constexpr bool debugMode               = false;
}