# TurnOffRGB

A minimal Windows app that detects every RGB device supported by [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB)
and turns its lighting off. No profiles, no effects, no network SDK, no plugins, no GUI beyond a tray icon.

## What it does

On launch, it detects all connected RGB hardware (using OpenRGB's device support library) and turns each
device's lighting off — using the device's own "Off" mode where one exists, otherwise by setting every LED
to black.

## Options (tray icon menu)

* **Start with Windows** — adds/removes a shortcut in the Windows Startup folder.
* **Stay running and keep RGB off** — when enabled, the app stays resident with a tray icon after turning
  lighting off, so it can be re-triggered or quit manually. When disabled, it turns lighting off and exits
  immediately.
* **Turn Off RGB Now** — re-applies the off state on demand (useful if other software turned lighting back on).
* **Exit**

## Building

Requires Qt (Widgets) and a Windows C++ toolchain. Open `TurnOffRGB.pro` in Qt Creator, or build with `qmake`
followed by `nmake`/`mingw32-make`.

## Credit

This project is a stripped-down fork of [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) by
CalcProgrammer1 and contributors — all device detection and control logic comes from that project.
See [LICENSE](LICENSE) (GPL-2.0-or-later).
