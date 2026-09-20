#include "inject.h"
#include "log.h"
#include <windows.h>
#include <tlhelp32.h>
#include <memory>
#include <string>

namespace {

void closeHandle(HANDLE handle) {
    if (handle) CloseHandle(handle);
}

using ScopedHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&closeHandle)>;

bool isTargetProcess(unsigned long processId, const wchar_t* name) {
    HANDLE rawProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!rawProcess) return false;
    ScopedHandle process(rawProcess, closeHandle);

    wchar_t imagePath[MAX_PATH];
    DWORD size = MAX_PATH;
    if (!QueryFullProcessImageNameW(process.get(), 0, imagePath, &size)) return false;

    size_t lastSeparator = std::wstring(imagePath).find_last_of(L"\\/");
    const wchar_t* fileName = (lastSeparator == std::wstring::npos) ? imagePath : imagePath + lastSeparator + 1;
    return _wcsicmp(fileName, name) == 0;
}

std::wstring toWide(const std::string& text) {
    int size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring wide(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wide[0], size);
    return wide;
}

void* writeRemoteString(HANDLE process, const std::wstring& text) {
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    void* remote = VirtualAllocEx(process, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) return nullptr;
    if (!WriteProcessMemory(process, remote, text.c_str(), bytes, nullptr)) {
        VirtualFreeEx(process, remote, 0, MEM_RELEASE);
        return nullptr;
    }
    return remote;
}

}

namespace inject {

Result inject(unsigned long processId, const std::string& dllPath, const wchar_t* targetName) {
    if (!isTargetProcess(processId, targetName)) return Result::TARGET_NOT_FOUND;

    ScopedHandle process(OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, processId), closeHandle);
    if (!process) {
        logDebug(("OpenProcess failed: " + std::to_string(GetLastError())).c_str());
        return Result::FAILED;
    }

    void* remotePath = writeRemoteString(process.get(), toWide(dllPath));
    if (!remotePath) {
        logDebug(("writing dll path to target failed: " + std::to_string(GetLastError())).c_str());
        return Result::FAILED;
    }

    FARPROC loadLibraryW = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    HANDLE rawThread = CreateRemoteThread(process.get(), nullptr, 0,
        (LPTHREAD_START_ROUTINE)loadLibraryW, remotePath, 0, nullptr);
    if (!rawThread) {
        logDebug(("CreateRemoteThread failed: " + std::to_string(GetLastError())).c_str());
        VirtualFreeEx(process.get(), remotePath, 0, MEM_RELEASE);
        return Result::FAILED;
    }
    ScopedHandle thread(rawThread, closeHandle);

    if (WaitForSingleObject(thread.get(), 10000) == WAIT_TIMEOUT) {
        logDebug("injection thread timed out (dll may have hung)");
        return Result::FAILED;
    }

    DWORD exitCode = 0;
    GetExitCodeThread(thread.get(), &exitCode);
    VirtualFreeEx(process.get(), remotePath, 0, MEM_RELEASE);

    if (exitCode == 0) {
        logDebug("LoadLibraryW failed inside the target");
        return Result::FAILED;
    }

    logDebug("dll loaded in target");
    return Result::OK;
}

bool findPID(const wchar_t* name, unsigned long& processId) {
    HANDLE rawSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (rawSnapshot == INVALID_HANDLE_VALUE) return false;
    ScopedHandle snapshot(rawSnapshot, closeHandle);

    PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
    if (!Process32FirstW(snapshot.get(), &entry)) return false;

    do {
        if (_wcsicmp(entry.szExeFile, name) == 0) {
            processId = entry.th32ProcessID;
            return true;
        }
    } while (Process32NextW(snapshot.get(), &entry));

    return false;
}

}