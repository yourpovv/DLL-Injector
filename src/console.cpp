#include "console.h"
#include <windows.h>

void setupConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleA("DLL Injector");

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) return;
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

int getWidth() {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) return 80;

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(handle, &info)) return 80;
    return info.srWindow.Right - info.srWindow.Left + 1;
}