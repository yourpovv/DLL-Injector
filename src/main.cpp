#include "banner.h"
#include "config.h"
#include "console.h"
#include "inject.h"
#include "log.h"
#include "theme.h"
#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool fileExists(const char* path) {
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

bool hasDllExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return false;
    std::string ext = path.substr(dot);
    for (char& ch : ext) ch = (char)std::tolower((unsigned char)ch);
    return ext == ".dll";
}

std::string dllPathFromCurrentDirectory(const std::string& dllName) {
    char directory[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, directory);
    return std::string(directory) + "\\" + dllName;
}

std::vector<std::string> findDllsInCurrentDirectory() {
    std::vector<std::string> dlls;
    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA("*.dll", &data);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) dlls.push_back(data.cFileName);
        } while (FindNextFileA(handle, &data));
        FindClose(handle);
    }
    return dlls;
}

std::string toNarrow(const std::wstring& wide) {
    std::string narrow;
    narrow.reserve(wide.size());
    for (wchar_t ch : wide) narrow.push_back((char)ch);
    return narrow;
}

void pressEnter(const char* prompt) {
    std::printf("\n");
    std::printf(WHITE "%s" RESET, prompt);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

bool pickTarget(std::wstring& targetName) {
    targetName = cfg::targetProcess;
    if (!targetName.empty()) return true;

    std::printf(WHITE "Target process name: " RESET);
    std::fflush(stdout);
    std::string line;
    std::getline(std::cin, line);
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
    if (line.empty()) {
        logLine("ERROR", "No process name given.");
        pressEnter("Press Enter to go back...");
        return false;
    }
    targetName.assign(line.begin(), line.end());
    return true;
}

void waitForTarget(DWORD& PID, const std::wstring& targetName) {
    logInfo(("Waiting for " + toNarrow(targetName) + "...").c_str());
    for (;;) {
        if (inject::findPID(targetName.c_str(), PID)) return;
        Sleep(1000);
    }
}

void InjectDLL(const std::string& droppedDllPath) {
    std::printf("\n");

    std::string dllName;
    if (!droppedDllPath.empty()) {
        if (!hasDllExtension(droppedDllPath)) {
            logLine("ERROR", "That's not a .dll file. Drop a DLL onto the loader and try again.");
            pressEnter("Press Enter to go back...");
            return;
        }
        dllName = droppedDllPath;
    } else {
        std::vector<std::string> dlls = findDllsInCurrentDirectory();
        if (dlls.empty()) {
            logLine("ERROR", "No DLL was found. Drop a .dll onto the loader, or place it next to it, and try again.");
            pressEnter("Press Enter to go back...");
            return;
        }
        if (dlls.size() == 1) {
            dllName = dlls[0];
        } else {
            for (size_t i = 0; i < dlls.size(); i++) {
                std::printf(WHITE "[%d]" RESET " %s\n", (int)(i + 1), dlls[i].c_str());
            }
            std::printf(WHITE "Select a DLL: " RESET);
            std::fflush(stdout);
            std::string line;
            std::getline(std::cin, line);
            int index = std::atoi(line.c_str());
            if (index < 1 || index > (int)dlls.size()) {
                logLine("ERROR", "Invalid selection.");
                pressEnter("Press Enter to go back...");
                return;
            }
            dllName = dlls[(size_t)(index - 1)];
        }
    }

    bool isFullPath = dllName.find(':') != std::string::npos || dllName.find('\\') != std::string::npos;
    std::string dllPath = isFullPath ? dllName : dllPathFromCurrentDirectory(dllName);

    logInfo(("Checking " + dllName).c_str());
    logDebug(("dllPath: " + dllPath).c_str());
    if (!fileExists(dllPath.c_str())) {
        logLine("ERROR", "The DLL wasn't found. Drop it onto the loader, or place it next to it, and try again.");
        pressEnter("Press Enter to go back...");
        return;
    }

    std::wstring targetName;
    if (!pickTarget(targetName)) return;

    DWORD PID = 0;
    waitForTarget(PID, targetName);
    logDebug(("target PID: " + std::to_string(PID)).c_str());
    logAction("OK", ("Found " + toNarrow(targetName) + ", injecting...").c_str());

    inject::Result result = inject::inject(PID, dllPath, targetName.c_str());
    logDebug(("inject result: " + std::to_string((int)result)).c_str());
    if (result == inject::Result::OK) {
        logAction("OK", "Injected");
    } else if (result == inject::Result::TARGET_NOT_FOUND) {
        logLine("ERROR", "The target closed before injection. Go back and try again.");
    } else {
        logLine("ERROR", "Injection failed. Disable your antivirus and try again.");
    }
    pressEnter("Press Enter to go back...");
}

void showMenu() {
    system("cls");
    banner::show();
    std::printf(WHITE "[1]" RESET GREEN " Start" RESET "\n");
    std::printf(WHITE "[0] Exit" RESET "\n");
    std::printf("\n");
}

char readChoice() {
    std::printf(WHITE "Select an option: " RESET);
    char line[16];
    if (!std::fgets(line, sizeof(line), stdin)) return '\0';
    return line[0];
}

}

int main(int argc, char* argv[]) {
    setupConsole();
    logDebug("Debug mode enabled");

    std::string droppedDllPath;
    if (argc > 1) {
        droppedDllPath = argv[1];
        logDebug(("dropped dll: " + droppedDllPath).c_str());
    }

    if (!droppedDllPath.empty()) {
        banner::show();
        std::printf("\n");
        InjectDLL(droppedDllPath);
        return 0;
    }

    for (;;) {
        showMenu();
        char choice = readChoice();
        if (choice == '1') InjectDLL("");
        else if (choice == '0') break;
    }

    return 0;
}