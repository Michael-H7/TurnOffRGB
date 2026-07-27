/*---------------------------------------------------------*\
| TurnOffRGB.cpp                                             |
|                                                             |
|   Entry point for TurnOffRGB - detects every RGB device    |
|   OpenRGB supports and turns its lighting off.  The tray   |
|   icon is implemented directly against the Win32 shell     |
|   APIs, no GUI toolkit required.                            |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cctype>
#include <cwchar>
#include <string>

#include "AutoStart.h"
#include "Colors.h"
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController.h"
#include "SettingsManager.h"
#include "resource.h"

static const char* SETTINGS_KEY = "TurnOffRGB";

static const UINT      WM_TRAYICON              = WM_APP + 1;
static const UINT_PTR  ID_START_WITH_WINDOWS    = 1001;
static const UINT_PTR  ID_STAY_RUNNING          = 1002;
static const UINT_PTR  ID_TURN_OFF_NOW          = 1003;
static const UINT_PTR  ID_EXIT                  = 1004;

static AutoStart*       g_autostart = nullptr;
static NOTIFYICONDATAW  g_nid       = {};

/*---------------------------------------------------------*\
| Turn off every currently detected RGB controller.  If a   |
| device exposes a mode literally named "Off", use it, as   |
| that is the only way some devices fully power down their  |
| lighting.  Otherwise fall back to setting every LED black.|
\*---------------------------------------------------------*/
static void TurnOffAllRGB()
{
    std::vector<RGBController*>& controllers = ResourceManager::get()->GetRGBControllers();

    for(RGBController* controller : controllers)
    {
        int off_mode = -1;

        for(unsigned int mode_idx = 0; mode_idx < controller->GetModeCount(); mode_idx++)
        {
            std::string mode_name = controller->GetModeName(mode_idx);

            std::transform(mode_name.begin(), mode_name.end(), mode_name.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });

            if(mode_name == "off")
            {
                off_mode = (int)mode_idx;
                break;
            }
        }

        if(off_mode >= 0)
        {
            controller->SetActiveMode(off_mode);
            controller->DeviceUpdateMode();
        }
        else
        {
            /*-------------------------------------------------------------*\
            | No explicit "Off" mode.  Setting colors alone is not enough - |
            | if the currently active mode is an on-device effect (e.g.    |
            | "Rainbow") it ignores the color buffer entirely and keeps    |
            | animating regardless of what we write.  SetCustomMode()      |
            | switches to a Direct/Custom/Static mode that actually honors |
            | per-LED colors before we black them out.                     |
            \*-------------------------------------------------------------*/
            controller->SetCustomMode();
            controller->DeviceUpdateMode();
            controller->SetAllColors(COLOR_BLACK);
            controller->DeviceUpdateLEDs();
        }
    }
}

static bool GetStayResidentSetting()
{
    json settings = ResourceManager::get()->GetSettingsManager()->GetSettings(SETTINGS_KEY);

    /*-------------------------------------------------------------*\
    | Default to staying resident until a setting is saved.  The    |
    | tray menu (with its own "stay running" toggle) is otherwise   |
    | unreachable on a fresh install: it only exists while the app  |
    | is staying resident in the first place.                       |
    \*-------------------------------------------------------------*/
    if(!settings.contains("stay_resident"))
    {
        return(true);
    }

    return(settings["stay_resident"]);
}

static void SetStayResidentSetting(bool stay_resident)
{
    SettingsManager* settings_manager = ResourceManager::get()->GetSettingsManager();
    json             settings         = settings_manager->GetSettings(SETTINGS_KEY);

    settings["stay_resident"] = stay_resident;

    settings_manager->SetSettings(SETTINGS_KEY, settings);
    settings_manager->SaveSettings();
}

static void ToggleAutoStart()
{
    if(g_autostart->IsAutoStartEnabled())
    {
        g_autostart->DisableAutoStart();
    }
    else
    {
        AutoStartInfo info;
        info.path = g_autostart->GetExePath();
        info.desc = "Turn Off RGB";

        g_autostart->EnableAutoStart(info);
    }
}

static void ShowTrayMenu(HWND hwnd)
{
    HMENU menu = CreatePopupMenu();

    AppendMenuW(menu, MF_STRING | (g_autostart->IsAutoStartEnabled() ? MF_CHECKED : MF_UNCHECKED),
        ID_START_WITH_WINDOWS, L"Start with Windows");
    AppendMenuW(menu, MF_STRING | (GetStayResidentSetting() ? MF_CHECKED : MF_UNCHECKED),
        ID_STAY_RUNNING, L"Stay running and keep RGB off");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TURN_OFF_NOW, L"Turn Off RGB Now");
    AppendMenuW(menu, MF_STRING, ID_EXIT, L"Exit");

    POINT cursor_pos;
    GetCursorPos(&cursor_pos);

    /*-----------------------------------------------------*\
    | Required so the popup menu closes correctly when the  |
    | user clicks away from it                              |
    \*-----------------------------------------------------*/
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, cursor_pos.x, cursor_pos.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);

    DestroyMenu(menu);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch(msg)
    {
        case WM_TRAYICON:
            if((LOWORD(lparam) == WM_RBUTTONUP) || (LOWORD(lparam) == WM_LBUTTONUP))
            {
                ShowTrayMenu(hwnd);
            }
            return 0;

        case WM_COMMAND:
            switch(LOWORD(wparam))
            {
                case ID_START_WITH_WINDOWS:
                    ToggleAutoStart();
                    break;

                case ID_STAY_RUNNING:
                    SetStayResidentSetting(!GetStayResidentSetting());
                    break;

                case ID_TURN_OFF_NOW:
                    TurnOffAllRGB();
                    break;

                case ID_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;

        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static int RunTrayApp(HINSTANCE instance)
{
    const wchar_t* CLASS_NAME = L"TurnOffRGBTrayWindow";

    WNDCLASSEXW wc      = {};
    wc.cbSize           = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = instance;
    wc.lpszClassName    = CLASS_NAME;

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"TurnOffRGB", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, instance, NULL);

    g_nid.cbSize            = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd              = hwnd;
    g_nid.uID               = 1;
    g_nid.uFlags            = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage  = WM_TRAYICON;
    g_nid.hIcon             = LoadIconW(instance, MAKEINTRESOURCEW(IDI_TRAYICON));
    wcscpy_s(g_nid.szTip, L"Turn Off RGB");

    Shell_NotifyIconW(NIM_ADD, &g_nid);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int)
{
    AutoStart autostart("TurnOffRGB");
    g_autostart = &autostart;

    /*-----------------------------------------------------*\
    | Detect every supported device, then immediately turn  |
    | its lighting off                                       |
    \*-----------------------------------------------------*/
    ResourceManager::get()->Initialize();
    ResourceManager::get()->WaitForDetection();

    TurnOffAllRGB();

    int result = 0;

    if(GetStayResidentSetting())
    {
        result = RunTrayApp(instance);
    }

    DetectionManager::get()->Cleanup();

    return result;
}
