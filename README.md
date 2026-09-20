# DLL-Injector

**Windows DLL injector with drag & drop support**

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![Language](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)

<img width="979" height="512" alt="preview" src="https://github.com/user-attachments/assets/3941927d-ecd4-4ea0-9f30-86774bae8e87" />
<div align="center">

</div>

## Features

- Works on any x64 game/process
- Drag & drop a `.dll` onto the exe to inject
- It auto detects dlls in the same path and asks which one if there are multiple
- Checks dll existance
- Ability to type the target game/proces name at launch, or hardcode it in `include/config.h`
- User mode injection with `LoadLibraryW` (`VirtualAllocEx` + `CreateRemoteThread`)
- Debug logging if you want it (`cfg::debugMode` in `include/config.h`) (idk why ppl dont release debugs anymore)

## Build

You need **Visual Studio** with **Desktop development with C++**

You can do either
- run `build.bat` and it builds to `build\DLL Injector.exe`
- open `DLL Injector.slnx` in Visual Studio and build

## Usage

1. Run the exe or drop your `.dll` on it
2. Pick `[1] Start`
3. Type the name of the game process to inject into (e.g. `VALORANT-Win64-Shipping.exe`)
4. Start that game

## Config

Settings are in `include/config.h`

| Setting         | Description                                              |
|-----------------|----------------------------------------------------------|
| `targetProcess` | process name to wait for, leave empty to type it at run  |
| `debugMode`     | set to `true` for `[DBG]` logging                        |

## Limits

- This is an x64 build so it only works with 64-bit processes
- Games protected by kernel anticheat (Vanguard, EAC, BattlEye etc) will block injection but unprotected games work fine

## Disclaimer

This is for educational purposes only. Injecting into processes you don't own can break their terms of service or the law. Use at your own risk.

## License

[MIT](LICENSE) © [YourPOV](https://github.com/yourpov)
