# TurnOffRGB

A minimal Windows app that detects every RGB device supported by [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB)
and turns its lighting off. No profiles, no effects, no network SDK, no plugins, no Qt — just a native tray icon.

## What it does

On launch, it detects all connected RGB hardware (using OpenRGB's device support library) and turns each
device's lighting off — using the device's own "Off" mode where one exists, otherwise by switching to a
Direct/Static mode and setting every LED to black (needed because some devices ignore color changes while
running an on-device effect like a default "Rainbow" cycle).

On a fresh install it stays resident with a tray icon so you can reach the menu below; once you've set your
preference it remembers it.

## Options (tray icon menu)

* **Start with Windows** — adds/removes a shortcut in the Windows Startup folder.
* **Stay running and keep RGB off** — when enabled, the app stays resident with a tray icon after turning
  lighting off, so it can be re-triggered or quit manually. When disabled, it turns lighting off and exits
  immediately on every launch.
* **Turn Off RGB Now** — re-applies the off state on demand (useful if other software turned lighting back on).
* **Exit**

### Motherboard / SMBus lighting not turning off?

Some onboard RGB (e.g. most motherboard headers) is controlled through the Super I/O chip or SMBus, which
needs kernel-level access. If those lights stay on, try running `TurnOffRGB.exe` as **Administrator** —
the first elevated run may also prompt to install the bundled PawnIO driver, which is what grants that access.

## Getting it

Every push builds automatically — grab the latest zip from the [Actions tab](../../actions/workflows/build.yml)
(open the most recent successful run and download the `TurnOffRGB-windows-x64` artifact), or build it yourself
below.

Unzip it to a **permanent folder** (e.g. `C:\Tools\TurnOffRGB\`), not somewhere you'll clean up later — the
"Start with Windows" option creates a shortcut pointing at wherever you run it from, and moving/deleting that
folder afterward will break it until you re-toggle the setting from the new location.

## Building

Requires:

* A Windows C++ toolchain — Visual Studio 2022 (Community is fine) or the standalone
  [Build Tools for Visual Studio 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
  with the "Desktop development with C++" workload.
* CMake 3.21+ (bundled with the Visual Studio installer above if you tick the CMake component, or install
  separately).

No other dependencies to install — everything else needed (hidapi, libusb, mbedtls, etc.) is vendored under
`dependencies/`.

**VS Code**: open the folder, install the recommended extensions (CMake Tools), then Ctrl+Shift+B. Use
*Terminal → Run Task → "Clean TurnOffRGB Build"* first if you want to force a from-scratch build.

**Command line**, from a Developer Command Prompt or with CMake/Visual Studio otherwise on PATH:

```bat
cmake --preset windows
cmake --build --preset windows-release
```

Either way, the result is `build\Release\TurnOffRGB.exe` plus the DLLs and PawnIO `.bin` files it needs
alongside it in that same folder — copy the whole folder's contents to wherever you're installing it (see
"Getting it" above).

## Credit

This project is a stripped-down fork of [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) by
CalcProgrammer1 and contributors — all device detection and control logic comes from that project.
See [LICENSE](LICENSE) (GPL-2.0-or-later).
