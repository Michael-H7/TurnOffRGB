#-----------------------------------------------------------------------------------------------#
# TurnOffRGB QMake Project                                                                      #
#                                                                                                #
#   Minimal, Windows-only fork of OpenRGB that detects every supported RGB device and turns its #
#   lighting off.  Built on top of OpenRGB's device detection and RGBController layer; the GUI, #
#   SDK network server/client, plugin system, profile system, and CLI have been removed.        #
#-----------------------------------------------------------------------------------------------#
QT += core gui widgets

CONFIG += c++17 silent

TARGET      = TurnOffRGB
TEMPLATE    = app

VERSION_STRING          = 1.0
GIT_COMMIT_ID           = $$system(git log -n 1 --pretty=format:"%H")
GIT_COMMIT_DATE         = $$system(git log -n 1 --pretty=format:"%ci")

DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STRING\\"\"\"                                               \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"

#-----------------------------------------------------------------------------------------------#
# Controller sources are dynamically gathered from OpenRGB's device support tree.  Only the      #
# generic and _Windows-suffixed files are used; _Linux/_FreeBSD/_MacOS files are excluded.       #
#-----------------------------------------------------------------------------------------------#
CONTROLLER_H            = $$files("Controllers/*.h",   true)
CONTROLLER_CPP          = $$files("Controllers/*.cpp", true)

for(iter, $$list($$CONTROLLER_H)) {
    CONTROLLER_INCLUDES += $$dirname(iter)
}
CONTROLLER_INCLUDES     = $$unique(CONTROLLER_INCLUDES)

CONTROLLER_H_WINDOWS    = $$files("Controllers/*_Windows*.h",   true)
CONTROLLER_CPP_WINDOWS  = $$files("Controllers/*_Windows*.cpp", true)
CONTROLLER_H_LINUX      = $$files("Controllers/*_Linux*.h",     true)
CONTROLLER_CPP_LINUX    = $$files("Controllers/*_Linux*.cpp",   true)
CONTROLLER_H_FREEBSD    = $$files("Controllers/*_FreeBSD*.h",   true)
CONTROLLER_CPP_FREEBSD  = $$files("Controllers/*_FreeBSD*.cpp", true)
CONTROLLER_H_MACOS      = $$files("Controllers/*_MacOS*.h",     true)
CONTROLLER_CPP_MACOS    = $$files("Controllers/*_MacOS*.cpp",   true)

CONTROLLER_H           -= $$CONTROLLER_H_WINDOWS
CONTROLLER_H           -= $$CONTROLLER_H_LINUX
CONTROLLER_H           -= $$CONTROLLER_H_FREEBSD
CONTROLLER_H           -= $$CONTROLLER_H_MACOS

CONTROLLER_CPP         -= $$CONTROLLER_CPP_WINDOWS
CONTROLLER_CPP         -= $$CONTROLLER_CPP_LINUX
CONTROLLER_CPP         -= $$CONTROLLER_CPP_FREEBSD
CONTROLLER_CPP         -= $$CONTROLLER_CPP_MACOS

HID_HOTPLUG_ENABLED = "true"

#-----------------------------------------------------------------------------------------------#
# Common configuration                                                                          #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    $$CONTROLLER_INCLUDES                                                                       \
    dependencies/ColorWheel                                                                     \
    dependencies/CRCpp/                                                                         \
    dependencies/hueplusplus-1.2.0/include                                                      \
    dependencies/hueplusplus-1.2.0/include/hueplusplus                                          \
    dependencies/httplib                                                                        \
    dependencies/json/                                                                          \
    dependencies/mdns                                                                           \
    dmiinfo/                                                                                    \
    hidapi_wrapper/                                                                             \
    i2c_smbus/                                                                                  \
    i2c_tools/                                                                                  \
    interop/                                                                                    \
    net_port/                                                                                   \
    pci_ids/                                                                                    \
    scsiapi/                                                                                    \
    serial_port/                                                                                \
    super_io/                                                                                   \
    AutoStart/                                                                                  \
    KeyboardLayoutManager/                                                                      \
    RGBController/                                                                              \
    SPDAccessor/                                                                                \
    dependencies/stb/                                                                           \
    dependencies/display-library/include                                                        \
    dependencies/hidapi-hotplug-win/include                                                     \
    dependencies/libusb-1.0.27/include                                                          \
    dependencies/mbedtls-3.2.1/include                                                          \
    dependencies/NVFC                                                                           \
    dependencies/PawnIO                                                                         \
    i2c_smbus/Windows                                                                           \
    wmi/

HEADERS +=                                                                                      \
    $$CONTROLLER_H                                                                              \
    $$CONTROLLER_H_WINDOWS                                                                      \
    Colors.h                                                                                    \
    dependencies/ColorWheel/ColorWheel.h                                                        \
    dependencies/json/nlohmann/json.hpp                                                         \
    JsonUtils.h                                                                                 \
    LogManager.h                                                                                \
    ResourceManager.h                                                                           \
    ResourceManagerCallback.h                                                                   \
    SettingsManager.h                                                                           \
    DetectionManager.h                                                                          \
    dmiinfo/dmiinfo.h                                                                            \
    filesystem.h                                                                                \
    hidapi_wrapper/hidapi_wrapper.h                                                             \
    i2c_smbus/i2c_smbus.h                                                                        \
    i2c_tools/i2c_tools.h                                                                        \
    interop/DeviceGuard.h                                                                       \
    interop/DeviceGuardLock.h                                                                   \
    interop/DeviceGuardManager.h                                                                \
    net_port/net_port.h                                                                          \
    pci_ids/pci_ids.h                                                                            \
    scsiapi/scsiapi.h                                                                            \
    serial_port/find_usb_serial_port.h                                                          \
    serial_port/serial_port.h                                                                    \
    super_io/super_io.h                                                                          \
    MathUtils.h                                                                                 \
    StringUtils.h                                                                               \
    AutoStart/AutoStart.h                                                                       \
    AutoStart/AutoStart-Windows.h                                                               \
    KeyboardLayoutManager/KeyboardLayoutManager.h                                               \
    RGBController/RGBController.h                                                               \
    RGBController/RGBController_Dummy.h                                                         \
    RGBController/RGBControllerKeyNames.h                                                       \
    RGBController/RGBController_Virtual.h                                                       \
    dependencies/display-library/include/adl_defines.h                                          \
    dependencies/display-library/include/adl_sdk.h                                              \
    dependencies/display-library/include/adl_structures.h                                       \
    dependencies/NVFC/nvapi.h                                                                   \
    dependencies/PawnIO/PawnIOLib.h                                                              \
    i2c_smbus/Windows/i2c_smbus_amdadl.h                                                        \
    i2c_smbus/Windows/i2c_smbus_nvapi.h                                                          \
    i2c_smbus/Windows/i2c_smbus_pawnio.h                                                        \
    wmi/wmi.h

SOURCES +=                                                                                      \
    $$CONTROLLER_CPP                                                                            \
    $$CONTROLLER_CPP_WINDOWS                                                                    \
    TurnOffRGB.cpp                                                                              \
    dependencies/ColorWheel/ColorWheel.cpp                                                      \
    dependencies/hueplusplus-1.2.0/src/Action.cpp                                               \
    dependencies/hueplusplus-1.2.0/src/APICache.cpp                                             \
    dependencies/hueplusplus-1.2.0/src/BaseDevice.cpp                                           \
    dependencies/hueplusplus-1.2.0/src/BaseHttpHandler.cpp                                      \
    dependencies/hueplusplus-1.2.0/src/Bridge.cpp                                               \
    dependencies/hueplusplus-1.2.0/src/BridgeConfig.cpp                                         \
    dependencies/hueplusplus-1.2.0/src/CLIPSensors.cpp                                          \
    dependencies/hueplusplus-1.2.0/src/ColorUnits.cpp                                           \
    dependencies/hueplusplus-1.2.0/src/EntertainmentMode.cpp                                    \
    dependencies/hueplusplus-1.2.0/src/ExtendedColorHueStrategy.cpp                             \
    dependencies/hueplusplus-1.2.0/src/ExtendedColorTemperatureStrategy.cpp                     \
    dependencies/hueplusplus-1.2.0/src/Group.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/HueCommandAPI.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/HueDeviceTypes.cpp                                       \
    dependencies/hueplusplus-1.2.0/src/HueException.cpp                                         \
    dependencies/hueplusplus-1.2.0/src/Light.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/ModelPictures.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/NewDeviceList.cpp                                        \
    dependencies/hueplusplus-1.2.0/src/Scene.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/Schedule.cpp                                             \
    dependencies/hueplusplus-1.2.0/src/Sensor.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/SimpleBrightnessStrategy.cpp                             \
    dependencies/hueplusplus-1.2.0/src/SimpleColorHueStrategy.cpp                               \
    dependencies/hueplusplus-1.2.0/src/SimpleColorTemperatureStrategy.cpp                       \
    dependencies/hueplusplus-1.2.0/src/StateTransaction.cpp                                     \
    dependencies/hueplusplus-1.2.0/src/TimePattern.cpp                                          \
    dependencies/hueplusplus-1.2.0/src/UPnP.cpp                                                 \
    dependencies/hueplusplus-1.2.0/src/Utils.cpp                                                \
    dependencies/hueplusplus-1.2.0/src/ZLLSensors.cpp                                           \
    dependencies/hueplusplus-1.2.0/src/WinHttpHandler.cpp                                       \
    dependencies/NVFC/nvapi.cpp                                                                 \
    DetectionManager.cpp                                                                        \
    dmiinfo/dmiinfo.cpp                                                                          \
    JsonUtils.cpp                                                                                \
    LogManager.cpp                                                                              \
    ResourceManager.cpp                                                                          \
    SPDAccessor/DDR4DirectAccessor.cpp                                                          \
    SPDAccessor/DDR5DirectAccessor.cpp                                                          \
    SPDAccessor/SPDAccessor.cpp                                                                 \
    SPDAccessor/SPDDetector.cpp                                                                 \
    SPDAccessor/SPDWrapper.cpp                                                                  \
    SettingsManager.cpp                                                                          \
    i2c_smbus/i2c_smbus.cpp                                                                      \
    i2c_smbus/Windows/i2c_smbus_amdadl.cpp                                                       \
    i2c_smbus/Windows/i2c_smbus_nvapi.cpp                                                        \
    i2c_tools/i2c_tools.cpp                                                                      \
    interop/DeviceGuard.cpp                                                                      \
    interop/DeviceGuardLock.cpp                                                                  \
    interop/DeviceGuardManager.cpp                                                               \
    net_port/net_port.cpp                                                                        \
    scsiapi/scsiapi_windows.c                                                                    \
    serial_port/serial_port.cpp                                                                  \
    serial_port/find_usb_serial_port_win.cpp                                                     \
    MathUtils.cpp                                                                                \
    StringUtils.cpp                                                                              \
    AutoStart/AutoStart.cpp                                                                      \
    AutoStart/AutoStart-Windows.cpp                                                              \
    KeyboardLayoutManager/KeyboardLayoutManager.cpp                                              \
    RGBController/RGBController.cpp                                                              \
    RGBController/RGBController_Dummy.cpp                                                        \
    RGBController/RGBControllerKeyNames.cpp                                                      \
    RGBController/RGBController_Virtual.cpp                                                      \
    wmi/wmi.cpp

RESOURCES +=                                                                                    \
    TurnOffRGB.qrc

#-----------------------------------------------------------------------------------------------#
# In-tree libe131 (E1.31/DMX) dependency                                                        #
#-----------------------------------------------------------------------------------------------#
SOURCES      += dependencies/libe131/src/e131.c
INCLUDEPATH  += dependencies/libe131/src/

#-----------------------------------------------------------------------------------------------#
# Windows Configuration                                                                         #
#-----------------------------------------------------------------------------------------------#
QMAKE_CXXFLAGS += /utf-8

contains(QMAKE_TARGET.arch, x86_64) {
    SOURCES += \
        i2c_smbus/Windows/i2c_smbus_pawnio.cpp \
        super_io/super_io_pawnio.cpp

    LIBS += \
        -lws2_32 \
        -liphlpapi \
        -L"$$PWD/dependencies/libusb-1.0.27/VS2019/MS64/dll" -llibusb-1.0 \
        -L"$$PWD/dependencies/hidapi-hotplug-win/x64/" -lhidapi-hotplug \
        -L"$$PWD/dependencies/mbedtls-3.2.1/lib/x64/" -lmbedcrypto -lmbedtls -lmbedx509 \
        -L"$$PWD/dependencies/PawnIO/" -lPawnIOLib
}

contains(QMAKE_TARGET.arch, x86) {
    SOURCES += \
        super_io/super_io.cpp

    LIBS += \
        -lws2_32 \
        -liphlpapi \
        -L"$$PWD/dependencies/libusb-1.0.27/VS2019/MS32/dll" -llibusb-1.0 \
        -L"$$PWD/dependencies/hidapi-hotplug-win/x86/" -lhidapi-hotplug \
        -L"$$PWD/dependencies/mbedtls-3.2.1/lib/x86/" -lmbedcrypto -lmbedtls -lmbedx509
}

DEFINES -= \
    UNICODE

DEFINES += \
    _MBCS \
    WIN32 \
    _CRT_SECURE_NO_WARNINGS \
    _WINSOCK_DEPRECATED_NO_WARNINGS \
    WIN32_LEAN_AND_MEAN \
    HID_HOTPLUG_ENABLED=1

RC_ICONS += TurnOffRGB.ico

DISTFILES += \
    dependencies/PawnIO/modules/SmbusPIIX4.bin \
    dependencies/PawnIO/modules/SmbusI801.bin \
    dependencies/PawnIO/modules/LpcIO.bin

#-----------------------------------------------------------------------------------------------#
# Build output layout                                                                           #
#-----------------------------------------------------------------------------------------------#
CONFIG(debug, debug|release) {
    DESTDIR = debug
}

CONFIG(release, debug|release) {
    DESTDIR = release
}

OBJECTS_DIR = _intermediate_$$DESTDIR/.obj
MOC_DIR     = _intermediate_$$DESTDIR/.moc
RCC_DIR     = _intermediate_$$DESTDIR/.qrc
UI_DIR      = _intermediate_$$DESTDIR/.ui

#-----------------------------------------------------------------------------------------------#
# Copy runtime dependencies to output directory                                                 #
#-----------------------------------------------------------------------------------------------#
contains(QMAKE_TARGET.arch, x86_64) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/libusb-1.0.27/VS2019/MS64/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/hidapi-hotplug-win/x64/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/PawnIOLib.dll                        )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusPIIX4.bin               )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusI801.bin                )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusIntelSkylakeIMC.bin     )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/SmbusNCT6793.bin             )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/PawnIO/modules/LpcIO.bin                    )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}

contains(QMAKE_TARGET.arch, x86) {
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/libusb-1.0.27/VS2019/MS32/dll/libusb-1.0.dll)\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    copydata.commands += $(COPY_FILE) \"$$shell_path($$PWD/dependencies/hidapi-hotplug-win/x86/hidapi-hotplug.dll   )\" \"$$shell_path($$DESTDIR)\" $$escape_expand(\n\t)
    first.depends = $(first) copydata
    export(first.depends)
    export(copydata.commands)
    QMAKE_EXTRA_TARGETS += first copydata
}
