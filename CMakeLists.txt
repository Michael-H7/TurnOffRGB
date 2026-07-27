#-----------------------------------------------------------------------------------------------#
# TurnOffRGB CMake project                                                                      #
#                                                                                                #
#   Minimal, Windows-only fork of OpenRGB that detects every supported RGB device and turns its #
#   lighting off.  Built on top of OpenRGB's device detection and RGBController layer.  No Qt,  #
#   no vcpkg required -- every third-party dependency ships vendored under dependencies/, the    #
#   same way the original qmake project consumed them.                                          #
#-----------------------------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.21)

project(TurnOffRGB LANGUAGES CXX C)

if(NOT WIN32)
    message(FATAL_ERROR "TurnOffRGB is Windows-only")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(RC)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TORGB_ARCH "x64")
else()
    set(TORGB_ARCH "x86")
endif()

#-------------------------------------------------------------------------------------------#
# Version / git info (best-effort; falls back quietly if git isn't available)               #
#-------------------------------------------------------------------------------------------#
set(TORGB_VERSION "1.0")
set(TORGB_GIT_COMMIT_ID "unknown")
set(TORGB_GIT_COMMIT_DATE "unknown")

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:%H
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE TORGB_GIT_COMMIT_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:%ci
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE TORGB_GIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()

#-------------------------------------------------------------------------------------------#
# Device controller sources -- gathered the same way the qmake project did: every file under #
# Controllers/, excluding files suffixed for other platforms (_Linux/_FreeBSD/_MacOS).       #
# CONFIGURE_DEPENDS re-runs the glob on the next configure if files are added/removed.       #
#-------------------------------------------------------------------------------------------#
file(GLOB_RECURSE CONTROLLER_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Controllers/*.cpp")
file(GLOB_RECURSE CONTROLLER_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Controllers/*.h")

list(FILTER CONTROLLER_SOURCES EXCLUDE REGEX "_Linux|_FreeBSD|_MacOS")
list(FILTER CONTROLLER_HEADERS EXCLUDE REGEX "_Linux|_FreeBSD|_MacOS")

set(CONTROLLER_INCLUDE_DIRS "")
foreach(header ${CONTROLLER_HEADERS})
    get_filename_component(header_dir ${header} DIRECTORY)
    list(APPEND CONTROLLER_INCLUDE_DIRS ${header_dir})
endforeach()
list(REMOVE_DUPLICATES CONTROLLER_INCLUDE_DIRS)

#-------------------------------------------------------------------------------------------#
# TurnOffRGB app + OpenRGB core (detection, RGBController, settings/log, hardware access)    #
#-------------------------------------------------------------------------------------------#
add_executable(TurnOffRGB WIN32
    TurnOffRGB.cpp
    TurnOffRGB.rc
    resource.h

    ${CONTROLLER_SOURCES}
    ${CONTROLLER_HEADERS}

    Colors.h
    DetectionManager.cpp
    DetectionManager.h
    JsonUtils.cpp
    JsonUtils.h
    LogManager.cpp
    LogManager.h
    MathUtils.cpp
    MathUtils.h
    ResourceManager.cpp
    ResourceManager.h
    ResourceManagerCallback.h
    SettingsManager.cpp
    SettingsManager.h
    StringUtils.cpp
    StringUtils.h
    filesystem.h

    AutoStart/AutoStart.cpp
    AutoStart/AutoStart.h
    AutoStart/AutoStart-Windows.cpp
    AutoStart/AutoStart-Windows.h

    KeyboardLayoutManager/KeyboardLayoutManager.cpp
    KeyboardLayoutManager/KeyboardLayoutManager.h

    RGBController/RGBController.cpp
    RGBController/RGBController.h
    RGBController/RGBController_Dummy.cpp
    RGBController/RGBController_Dummy.h
    RGBController/RGBControllerKeyNames.cpp
    RGBController/RGBControllerKeyNames.h
    RGBController/RGBController_Virtual.cpp
    RGBController/RGBController_Virtual.h

    SPDAccessor/DDR4DirectAccessor.cpp
    SPDAccessor/DDR5DirectAccessor.cpp
    SPDAccessor/SPDAccessor.cpp
    SPDAccessor/SPDDetector.cpp
    SPDAccessor/SPDWrapper.cpp

    dmiinfo/dmiinfo.cpp
    dmiinfo/dmiinfo.h
    hidapi_wrapper/hidapi_wrapper.h
    i2c_smbus/i2c_smbus.cpp
    i2c_smbus/i2c_smbus.h
    i2c_smbus/Windows/i2c_smbus_amdadl.cpp
    i2c_smbus/Windows/i2c_smbus_amdadl.h
    i2c_smbus/Windows/i2c_smbus_nvapi.cpp
    i2c_smbus/Windows/i2c_smbus_nvapi.h
    i2c_smbus/Windows/i2c_smbus_pawnio.h
    i2c_tools/i2c_tools.cpp
    i2c_tools/i2c_tools.h
    interop/DeviceGuard.cpp
    interop/DeviceGuard.h
    interop/DeviceGuardLock.cpp
    interop/DeviceGuardLock.h
    interop/DeviceGuardManager.cpp
    interop/DeviceGuardManager.h
    net_port/net_port.cpp
    net_port/net_port.h
    pci_ids/pci_ids.h
    scsiapi/scsiapi.h
    scsiapi/scsiapi_windows.c
    serial_port/find_usb_serial_port.h
    serial_port/find_usb_serial_port_win.cpp
    serial_port/serial_port.cpp
    serial_port/serial_port.h
    super_io/super_io.h
    wmi/wmi.cpp
    wmi/wmi.h

    dependencies/NVFC/nvapi.cpp
    dependencies/NVFC/nvapi.h
    dependencies/libe131/src/e131.c

    dependencies/hueplusplus-1.2.0/src/Action.cpp
    dependencies/hueplusplus-1.2.0/src/APICache.cpp
    dependencies/hueplusplus-1.2.0/src/BaseDevice.cpp
    dependencies/hueplusplus-1.2.0/src/BaseHttpHandler.cpp
    dependencies/hueplusplus-1.2.0/src/Bridge.cpp
    dependencies/hueplusplus-1.2.0/src/BridgeConfig.cpp
    dependencies/hueplusplus-1.2.0/src/CLIPSensors.cpp
    dependencies/hueplusplus-1.2.0/src/ColorUnits.cpp
    dependencies/hueplusplus-1.2.0/src/EntertainmentMode.cpp
    dependencies/hueplusplus-1.2.0/src/ExtendedColorHueStrategy.cpp
    dependencies/hueplusplus-1.2.0/src/ExtendedColorTemperatureStrategy.cpp
    dependencies/hueplusplus-1.2.0/src/Group.cpp
    dependencies/hueplusplus-1.2.0/src/HueCommandAPI.cpp
    dependencies/hueplusplus-1.2.0/src/HueDeviceTypes.cpp
    dependencies/hueplusplus-1.2.0/src/HueException.cpp
    dependencies/hueplusplus-1.2.0/src/Light.cpp
    dependencies/hueplusplus-1.2.0/src/ModelPictures.cpp
    dependencies/hueplusplus-1.2.0/src/NewDeviceList.cpp
    dependencies/hueplusplus-1.2.0/src/Scene.cpp
    dependencies/hueplusplus-1.2.0/src/Schedule.cpp
    dependencies/hueplusplus-1.2.0/src/Sensor.cpp
    dependencies/hueplusplus-1.2.0/src/SimpleBrightnessStrategy.cpp
    dependencies/hueplusplus-1.2.0/src/SimpleColorHueStrategy.cpp
    dependencies/hueplusplus-1.2.0/src/SimpleColorTemperatureStrategy.cpp
    dependencies/hueplusplus-1.2.0/src/StateTransaction.cpp
    dependencies/hueplusplus-1.2.0/src/TimePattern.cpp
    dependencies/hueplusplus-1.2.0/src/UPnP.cpp
    dependencies/hueplusplus-1.2.0/src/Utils.cpp
    dependencies/hueplusplus-1.2.0/src/ZLLSensors.cpp
    dependencies/hueplusplus-1.2.0/src/WinHttpHandler.cpp
)

if(TORGB_ARCH STREQUAL "x64")
    target_sources(TurnOffRGB PRIVATE
        i2c_smbus/Windows/i2c_smbus_pawnio.cpp
        super_io/super_io_pawnio.cpp)
else()
    target_sources(TurnOffRGB PRIVATE
        super_io/super_io.cpp)
endif()

target_include_directories(TurnOffRGB PRIVATE
    ${CONTROLLER_INCLUDE_DIRS}
    dependencies/CRCpp
    dependencies/hueplusplus-1.2.0/include
    dependencies/hueplusplus-1.2.0/include/hueplusplus
    dependencies/httplib
    dependencies/json
    dependencies/mdns
    dependencies/stb
    dependencies/display-library/include
    dependencies/hidapi-hotplug-win/include
    dependencies/libusb-1.0.27/include
    dependencies/mbedtls-3.2.1/include
    dependencies/NVFC
    dependencies/PawnIO
    dependencies/libe131/src
    dmiinfo
    hidapi_wrapper
    i2c_smbus
    i2c_smbus/Windows
    i2c_tools
    interop
    net_port
    pci_ids
    scsiapi
    serial_port
    super_io
    wmi
    AutoStart
    KeyboardLayoutManager
    RGBController
    SPDAccessor
    ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(TurnOffRGB PRIVATE
    VERSION_STRING="${TORGB_VERSION}"
    GIT_COMMIT_ID="${TORGB_GIT_COMMIT_ID}"
    GIT_COMMIT_DATE="${TORGB_GIT_COMMIT_DATE}"
    _MBCS
    WIN32
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    HID_HOTPLUG_ENABLED=1
)

if(MSVC)
    target_compile_options(TurnOffRGB PRIVATE /utf-8)
endif()

if(TORGB_ARCH STREQUAL "x64")
    set(TORGB_LIBUSB_DIR   "${CMAKE_SOURCE_DIR}/dependencies/libusb-1.0.27/VS2019/MS64/dll")
    set(TORGB_HIDAPI_DIR   "${CMAKE_SOURCE_DIR}/dependencies/hidapi-hotplug-win/x64")
    set(TORGB_MBEDTLS_DIR  "${CMAKE_SOURCE_DIR}/dependencies/mbedtls-3.2.1/lib/x64")
else()
    set(TORGB_LIBUSB_DIR   "${CMAKE_SOURCE_DIR}/dependencies/libusb-1.0.27/VS2019/MS32/dll")
    set(TORGB_HIDAPI_DIR   "${CMAKE_SOURCE_DIR}/dependencies/hidapi-hotplug-win/x86")
    set(TORGB_MBEDTLS_DIR  "${CMAKE_SOURCE_DIR}/dependencies/mbedtls-3.2.1/lib/x86")
endif()

target_link_libraries(TurnOffRGB PRIVATE
    ws2_32
    iphlpapi
    "${TORGB_LIBUSB_DIR}/libusb-1.0.lib"
    "${TORGB_HIDAPI_DIR}/hidapi-hotplug.lib"
)

if(TORGB_ARCH STREQUAL "x64")
    target_link_libraries(TurnOffRGB PRIVATE
        "${TORGB_MBEDTLS_DIR}/mbedcrypto.lib"
        "${TORGB_MBEDTLS_DIR}/mbedtls.lib"
        "${TORGB_MBEDTLS_DIR}/mbedx509.lib"
        "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/PawnIOLib.lib"
    )
endif()

#-------------------------------------------------------------------------------------------#
# Copy runtime dependencies (DLLs + PawnIO modules) next to the built exe                    #
#-------------------------------------------------------------------------------------------#
add_custom_command(TARGET TurnOffRGB POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TORGB_LIBUSB_DIR}/libusb-1.0.dll"
        "${TORGB_HIDAPI_DIR}/hidapi-hotplug.dll"
        $<TARGET_FILE_DIR:TurnOffRGB>
)

if(TORGB_ARCH STREQUAL "x64")
    add_custom_command(TARGET TurnOffRGB POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/PawnIOLib.dll"
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/modules/SmbusPIIX4.bin"
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/modules/SmbusI801.bin"
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/modules/SmbusIntelSkylakeIMC.bin"
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/modules/SmbusNCT6793.bin"
            "${CMAKE_SOURCE_DIR}/dependencies/PawnIO/modules/LpcIO.bin"
            $<TARGET_FILE_DIR:TurnOffRGB>
    )
endif()
