/*---------------------------------------------------------*\
| TurnOffRGB.cpp                                             |
|                                                             |
|   Entry point for TurnOffRGB - detects every RGB device    |
|   OpenRGB supports and turns its lighting off.              |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cctype>
#include <string>

#include <QApplication>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

#include "AutoStart.h"
#include "Colors.h"
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController.h"
#include "SettingsManager.h"

static const char* SETTINGS_KEY = "TurnOffRGB";

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
            controller->SetAllColors(COLOR_BLACK);
            controller->DeviceUpdateLEDs();
        }
    }
}

static bool GetStayResidentSetting()
{
    json settings = ResourceManager::get()->GetSettingsManager()->GetSettings(SETTINGS_KEY);

    return(settings.contains("stay_resident") && settings["stay_resident"]);
}

static void SetStayResidentSetting(bool stay_resident)
{
    SettingsManager* settings_manager = ResourceManager::get()->GetSettingsManager();
    json             settings         = settings_manager->GetSettings(SETTINGS_KEY);

    settings["stay_resident"] = stay_resident;

    settings_manager->SetSettings(SETTINGS_KEY, settings);
    settings_manager->SaveSettings();
}

/*---------------------------------------------------------*\
| Tray icon exposing the app's only options.  Left as a     |
| plain QObject subclass (no Q_OBJECT/signals of our own)   |
| so no moc step is needed for this file.                   |
\*---------------------------------------------------------*/
class TurnOffRGBTray : public QObject
{
public:
    explicit TurnOffRGBTray(AutoStart& autostart_ref) :
        autostart(autostart_ref)
    {
        menu = new QMenu();

        start_with_windows_action = menu->addAction("Start with Windows");
        start_with_windows_action->setCheckable(true);
        start_with_windows_action->setChecked(autostart.IsAutoStartEnabled());
        connect(start_with_windows_action, &QAction::toggled, this, &TurnOffRGBTray::OnStartWithWindowsToggled);

        stay_running_action = menu->addAction("Stay running and keep RGB off");
        stay_running_action->setCheckable(true);
        stay_running_action->setChecked(GetStayResidentSetting());
        connect(stay_running_action, &QAction::toggled, this, &TurnOffRGBTray::OnStayRunningToggled);

        menu->addSeparator();

        QAction* turn_off_now_action = menu->addAction("Turn Off RGB Now");
        connect(turn_off_now_action, &QAction::triggered, this, [](){ TurnOffAllRGB(); });

        QAction* exit_action = menu->addAction("Exit");
        connect(exit_action, &QAction::triggered, qApp, &QCoreApplication::quit);

        tray_icon = new QSystemTrayIcon(QIcon(":/icons/TurnOffRGB.ico"));
        tray_icon->setToolTip("Turn Off RGB");
        tray_icon->setContextMenu(menu);
        tray_icon->show();
    }

    ~TurnOffRGBTray()
    {
        delete tray_icon;
        delete menu;
    }

private:
    void OnStartWithWindowsToggled(bool checked)
    {
        if(checked)
        {
            AutoStartInfo info;
            info.path = autostart.GetExePath();
            info.desc = "Turn Off RGB";

            autostart.EnableAutoStart(info);
        }
        else
        {
            autostart.DisableAutoStart();
        }
    }

    void OnStayRunningToggled(bool checked)
    {
        SetStayResidentSetting(checked);
    }

    AutoStart&          autostart;
    QMenu*              menu;
    QSystemTrayIcon*    tray_icon;
    QAction*            start_with_windows_action;
    QAction*            stay_running_action;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    QApplication::setOrganizationName("TurnOffRGB");
    QApplication::setApplicationName("TurnOffRGB");

    AutoStart autostart("TurnOffRGB");

    /*-----------------------------------------------------*\
    | Detect every supported device, then immediately turn  |
    | its lighting off                                       |
    \*-----------------------------------------------------*/
    ResourceManager::get()->Initialize();
    ResourceManager::get()->WaitForDetection();

    TurnOffAllRGB();

    int  result        = 0;
    bool stay_resident  = GetStayResidentSetting() && QSystemTrayIcon::isSystemTrayAvailable();

    if(stay_resident)
    {
        TurnOffRGBTray tray(autostart);

        result = app.exec();
    }

    DetectionManager::get()->Cleanup();

    return result;
}
