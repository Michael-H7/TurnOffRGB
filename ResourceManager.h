/*---------------------------------------------------------*\
| ResourceManager.h                                         |
|                                                           |
|   OpenRGB Resource Manager controls access to application |
|   components including RGBControllers and I2C interfaces  |
|                                                           |
|   Adam Honse (CalcProgrammer1)                27 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <thread>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "DetectionManager.h"
#include "hidapi_wrapper.h"
#include "i2c_smbus.h"
#include "ResourceManagerCallback.h"
#include "filesystem.h"
#include "find_usb_serial_port.h"

using json = nlohmann::json;

class LogManager;
class RGBController;
class SettingsManager;

typedef struct
{
    unsigned short  vendor_id;
    unsigned short  product_id;
    unsigned short  release_number;
    unsigned short  usage_page;
    unsigned short  usage;
    int             interface_number;
    std::string     serial_number;
    std::string     manufacturer_string;
    std::string     product_string;
    std::string     path;
}   HIDDeviceInfo;

typedef struct
{
    unsigned short  vendor_id;
    unsigned short  product_id;
    std::string     serial_number;
    std::string     manufacturer_string;
    std::string     product_string;
}   USBDeviceInfo;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    /*-----------------------------------------------------*\
    | ResourceManager Global Instance Accessor              |
    \*-----------------------------------------------------*/
    static ResourceManager*             get();

    /*-----------------------------------------------------*\
    | Resource Accessors                                    |
    \*-----------------------------------------------------*/
    filesystem::path                    GetConfigurationDirectory();
    LogManager*                         GetLogManager();
    std::vector<HIDDeviceInfo>          GetHIDDeviceInfo();
    std::vector<i2c_smbus_interface*>&  GetI2CBuses();
    std::vector<i2c_smbus_info>         GetI2CBusInfo();
    std::vector<std::string>            GetSerialPorts();
    std::vector<SerialDeviceInfo>       GetUSBSerialPorts();
    std::vector<USBDeviceInfo>          GetUSBDeviceInfo();
    std::vector<RGBController*>&        GetRGBControllers();
    std::vector<RGBControllerInterface*>&   GetRGBControllerInterfaces();
    SettingsManager*                    GetSettingsManager();

    void                                SetConfigurationDirectory(const filesystem::path &directory);

    /*-----------------------------------------------------*\
    | Callback Registration Functions                       |
    \*-----------------------------------------------------*/
    void                                RegisterResourceManagerCallback(ResourceManagerCallback new_callback, void * new_callback_arg);
    void                                UnregisterResourceManagerCallback(ResourceManagerCallback new_callback, void * new_callback_arg);

    /*-----------------------------------------------------*\
    | Functions to manage detection                         |
    \*-----------------------------------------------------*/
    unsigned int                        GetDetectionPercent();
    std::string                         GetDetectionString();
    void                                RescanDevices();
    void                                UpdateDeviceList();
    void                                WaitForDetection();

    /*-----------------------------------------------------*\
    | Functions to signal update callbacks                  |
    \*-----------------------------------------------------*/
    void                                SignalResourceManagerUpdate(unsigned int update_reason);

    void                                Initialize();

    void                                WaitForInitialization();

private:
    void                                SetupConfigurationDirectory();

    /*-----------------------------------------------------*\
    | Static pointer to shared instance of ResourceManager  |
    \*-----------------------------------------------------*/
    static ResourceManager*                     instance;

    /*-----------------------------------------------------*\
    | Settings Manager                                      |
    \*-----------------------------------------------------*/
    SettingsManager*                            settings_manager;

    /*-----------------------------------------------------*\
    | RGBControllers                                        |
    \*-----------------------------------------------------*/
    std::vector<RGBController*>                 rgb_controllers;
    std::vector<RGBControllerInterface*>        rgb_controller_interfaces;

    /*-----------------------------------------------------*\
    | Device List Mutex                                     |
    \*-----------------------------------------------------*/
    std::mutex                                  DeviceListChangeMutex;

    /*-----------------------------------------------------*\
    | Initialization completion flag                        |
    \*-----------------------------------------------------*/
    std::atomic<bool>                           init_finished;

    /*-----------------------------------------------------*\
    | ResourceManager Callbacks                             |
    \*-----------------------------------------------------*/
    std::vector<ResourceManagerCallback>        ResourceManagerCallbacks;
    std::vector<void *>                         ResourceManagerCallbackArgs;
    std::mutex                                  ResourceManagerCallbackMutex;

    /*-----------------------------------------------------*\
    | Configuration directory path                          |
    \*-----------------------------------------------------*/
    filesystem::path                            config_dir;
};
