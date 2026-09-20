@echo off
setlocal

set "ROOT=%~dp0"
set "BUILD=%ROOT%build"

if not exist "%BUILD%" mkdir "%BUILD%"

if not defined DevEnvDir (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
    )
    if not defined VSROOT (
        for %%p in (
            "C:\Program Files\Microsoft Visual Studio\18\Community"
            "C:\Program Files\Microsoft Visual Studio\2022\Community"
            "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
            "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
        ) do (
            if exist "%%~p\VC\Auxiliary\Build\vcvars64.bat" set "VSROOT=%%~p"
        )
    )
)

if not defined DevEnvDir (
    if not defined VSROOT (
        echo Visual Studio C++ tools not found. Install the "Desktop development with C++" workload.
        pause
        exit /b 1
    )
    call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
)

rc /nologo /fo"%BUILD%\resource.res" "%ROOT%resource.rc"
if errorlevel 1 (
    echo Resource compile failed.
    pause
    exit /b 1
)

cl /nologo /std:c++17 /O2 /EHsc /utf-8 /W4 /DUNICODE /D_UNICODE /I"%ROOT%include" ^
    "%ROOT%src\main.cpp" "%ROOT%src\banner.cpp" ^
    "%ROOT%src\console.cpp" "%ROOT%src\inject.cpp" ^
    "%ROOT%src\log.cpp" "%BUILD%\resource.res" ^
    /Fo"%BUILD%\\" /Fe:"%BUILD%\DLL Injector.exe" ^
    /link advapi32.lib /MANIFEST:NO

if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo %BUILD%\DLL Injector.exe