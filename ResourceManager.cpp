/*---------------------------------------------------------*\
| ResourceManager.cpp                                       |
|                                                           |
|   OpenRGB Resource Manager controls access to application |
|   components including RGBControllers and I2C interfaces  |
|                                                           |
|   Adam Honse (CalcProgrammer1)                27 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <stdlib.h>
#include <string>
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "LogManager.h"
#include "serial_port.h"
#include "SettingsManager.h"
#include "StringUtils.h"
#include "filesystem.h"

/*---------------------------------------------------------*\
| Put these last to avoid some include order issues         |
\*---------------------------------------------------------*/
#include <hidapi.h>
#include <libusb.h>

using namespace std::chrono_literals;

/*---------------------------------------------------------*\
| ResourceManager Callback Functions                        |
\*---------------------------------------------------------*/
static void ResourceManagerDetectionCallback(void * this_ptr, unsigned int update_reason)
{
    ResourceManager* this_obj = (ResourceManager *)this_ptr;

    switch(update_reason)
    {
        case DETECTIONMANAGER_UPDATE_REASON_I2C_BUS_REGISTERED:
            this_obj->SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_I2C_BUS_LIST_UPDATED);
            break;

        case DETECTIONMANAGER_UPDATE_REASON_DETECTION_STARTED:
            this_obj->SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_DETECTION_STARTED);
            break;

        case DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_REGISTERED:
        case DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_UNREGISTERED:
        case DETECTIONMANAGER_UPDATE_REASON_RGBCONTROLLER_LIST_CLEARED:
            this_obj->UpdateDeviceList();
            break;

        case DETECTIONMANAGER_UPDATE_REASON_DETECTION_PROGRESS_CHANGED:
            this_obj->SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_DETECTION_PROGRESS_CHANGED);
            break;

        case DETECTIONMANAGER_UPDATE_REASON_DETECTION_COMPLETE:
            this_obj->SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_DETECTION_COMPLETE);
            break;
    }
}

/*---------------------------------------------------------*\
| ResourceManager name for log entries                      |
\*---------------------------------------------------------*/
const char* RESOURCEMANAGER = "ResourceManager";

/*---------------------------------------------------------*\
| ResourceManager Global Instance Pointer                   |
\*---------------------------------------------------------*/
ResourceManager* ResourceManager::instance;

ResourceManager::ResourceManager()
{
    /*-----------------------------------------------------*\
    | Initialize global instance pointer the when created   |
    | There should only ever be one instance of             |
    | ResourceManager                                       |
    \*-----------------------------------------------------*/
    if(!instance)
    {
        instance = this;
    }
    /*-----------------------------------------------------*\
    | If, for whatever reason, ResourceManager already      |
    | exists, delete this instance as only one should exist |
    \*-----------------------------------------------------*/
    else
    {
        delete this;
        return;
    }

    /*-----------------------------------------------------*\
    | Initialize Detection Variables                        |
    \*-----------------------------------------------------*/
    init_finished               = false;

    SetupConfigurationDirectory();

    /*-----------------------------------------------------*\
    | Load settings from file                               |
    \*-----------------------------------------------------*/
    settings_manager            = new SettingsManager();

    settings_manager->LoadSettings(GetConfigurationDirectory() / "TurnOffRGB.json");

    /*-----------------------------------------------------*\
    | Create Detection settings schema                      |
    \*-----------------------------------------------------*/
    json                detection_settings_schema;

    detection_settings_schema["hid_safe_mode"]["title"]                     = "HID Safe Mode";
    detection_settings_schema["hid_safe_mode"]["type"]                      = "bool";
    detection_settings_schema["hid_safe_mode"]["description"]               = "Use an alternate method for detecting HID devices";

    detection_settings_schema["initial_detection_delay_ms"]["title"]        = "Initial Detection Delay (ms)";
    detection_settings_schema["initial_detection_delay_ms"]["type"]         = "integer";
    detection_settings_schema["initial_detection_delay_ms"]["description"]  = "Amount of time, in milliseconds, to wait before detecting devices when started";

    settings_manager->RegisterSettingsSchema("Detectors", "Detection", detection_settings_schema);

    /*-----------------------------------------------------*\
    | Create LogManager settings schema                     |
    \*-----------------------------------------------------*/
    json                logmanager_settings_schema;

    logmanager_settings_schema["log_console"]["title"]                      = "Enable Log Console";
    logmanager_settings_schema["log_console"]["type"]                       = "bool";

    logmanager_settings_schema["log_file"]["title"]                         = "Enable Log File";
    logmanager_settings_schema["log_file"]["type"]                          = "bool";
    logmanager_settings_schema["log_file"]["default"]                       = true;

    logmanager_settings_schema["loglevel"]["title"]                         = "Log Level";
    logmanager_settings_schema["loglevel"]["type"]                          = "integer";
    logmanager_settings_schema["loglevel"]["default"]                       = 3;
    logmanager_settings_schema["loglevel"]["enum"][0]                       = 0;
    logmanager_settings_schema["loglevel"]["enumNames"][0]                  = "Fatal";
    logmanager_settings_schema["loglevel"]["enum"][1]                       = 1;
    logmanager_settings_schema["loglevel"]["enumNames"][1]                  = "Error";
    logmanager_settings_schema["loglevel"]["enum"][2]                       = 2;
    logmanager_settings_schema["loglevel"]["enumNames"][2]                  = "Warning";
    logmanager_settings_schema["loglevel"]["enum"][3]                       = 3;
    logmanager_settings_schema["loglevel"]["enumNames"][3]                  = "Info";
    logmanager_settings_schema["loglevel"]["enum"][4]                       = 4;
    logmanager_settings_schema["loglevel"]["enumNames"][4]                  = "Verbose";
    logmanager_settings_schema["loglevel"]["enum"][5]                       = 5;
    logmanager_settings_schema["loglevel"]["enumNames"][5]                  = "Debug";
    logmanager_settings_schema["loglevel"]["enum"][6]                       = 6;
    logmanager_settings_schema["loglevel"]["enumNames"][6]                  = "Trace";

    logmanager_settings_schema["file_count_limit"]["title"]                 = "Log File Count Limit";
    logmanager_settings_schema["file_count_limit"]["type"]                  = "integer";
    logmanager_settings_schema["file_count_limit"]["description"]           = "Maximum number of log files to keep, 0 for no limit";
    logmanager_settings_schema["file_count_limit"]["default"]               = 10;
    logmanager_settings_schema["file_count_limit"]["minimum"]               = 0;

    settings_manager->RegisterSettingsSchema("LogManager", "Log Manager", logmanager_settings_schema);

    /*-----------------------------------------------------*\
    | Configure the log manager                             |
    \*-----------------------------------------------------*/
    LogManager::get()->Configure(settings_manager->GetSettings("LogManager"), GetConfigurationDirectory());
}

ResourceManager::~ResourceManager()
{

}

/*---------------------------------------------------------*\
| ResourceManager Global Instance Accessor                  |
\*---------------------------------------------------------*/
ResourceManager* ResourceManager::get()
{
    /*-----------------------------------------------------*\
    | If ResourceManager does not exist yet, create it      |
    \*-----------------------------------------------------*/
    if(!instance)
    {
        instance = new ResourceManager();
    }

    return instance;
}

/*---------------------------------------------------------*\
| Resource Accessors                                        |
\*---------------------------------------------------------*/
filesystem::path ResourceManager::GetConfigurationDirectory()
{
    return(config_dir);
}

std::vector<HIDDeviceInfo> ResourceManager::GetHIDDeviceInfo()
{
    hid_device_info*                    hid_devices;
    std::vector<HIDDeviceInfo>          hid_info;

    hid_devices = hid_enumerate(0,0);

    while(hid_devices)
    {
        HIDDeviceInfo                   hid_device_info;

        hid_device_info.vendor_id           = hid_devices->vendor_id;
        hid_device_info.product_id          = hid_devices->product_id;
        hid_device_info.release_number      = hid_devices->release_number;
        hid_device_info.usage_page          = hid_devices->usage_page;
        hid_device_info.usage               = hid_devices->usage;
        hid_device_info.interface_number    = hid_devices->interface_number;
        hid_device_info.serial_number       = StringUtils::wchar_to_string(hid_devices->serial_number);
        hid_device_info.manufacturer_string = StringUtils::wchar_to_string(hid_devices->manufacturer_string);
        hid_device_info.product_string      = StringUtils::wchar_to_string(hid_devices->product_string);
        hid_device_info.path                = hid_devices->path;

        hid_info.push_back(hid_device_info);

        hid_devices = hid_devices->next;
    }

    return(hid_info);
}

std::vector<i2c_smbus_interface*> & ResourceManager::GetI2CBuses()
{
    return DetectionManager::get()->GetI2CBuses();
}

std::vector<i2c_smbus_info> ResourceManager::GetI2CBusInfo()
{
    std::vector<i2c_smbus_info>         bus_info;
    std::vector<i2c_smbus_interface*>   buses       = GetI2CBuses();

    for(std::size_t bus_idx = 0; bus_idx < buses.size(); bus_idx++)
    {
        bus_info.push_back(buses[bus_idx]->info);
    }

    return(bus_info);
}

std::vector<std::string> ResourceManager::GetSerialPorts()
{
    return(serial_port::getSerialPorts());
}

std::vector<SerialDeviceInfo> ResourceManager::GetUSBSerialPorts()
{
    return(find_usb_serial_ports());
}

std::vector<USBDeviceInfo> ResourceManager::GetUSBDeviceInfo()
{
    std::size_t                         usb_device_count;
    libusb_device**                     usb_devices;
    std::vector<USBDeviceInfo>          usb_info;

    if(libusb_init(NULL) >= 0)
    {
        usb_device_count = libusb_get_device_list(NULL, &usb_devices);

        for(std::size_t usb_device_idx = 0; usb_device_idx < usb_device_count; usb_device_idx++)
        {
            libusb_device_descriptor    usb_desc;
            libusb_device_handle*       usb_handle;

            if(libusb_get_device_descriptor(usb_devices[usb_device_idx], &usb_desc) >= 0)
            {
                USBDeviceInfo           usb_device_info;
                char                    usb_descriptor_buf[256];

                usb_device_info.vendor_id   = usb_desc.idVendor;
                usb_device_info.product_id  = usb_desc.idProduct;

                if(libusb_open(usb_devices[usb_device_idx], &usb_handle) >= 0)
                {
                    if(usb_desc.iSerialNumber)
                    {
                        libusb_get_string_descriptor_ascii(usb_handle, usb_desc.iSerialNumber, (unsigned char*)usb_descriptor_buf, sizeof(usb_descriptor_buf));
                        usb_device_info.serial_number.assign(usb_descriptor_buf);
                    }

                    if(usb_desc.iManufacturer)
                    {
                        libusb_get_string_descriptor_ascii(usb_handle, usb_desc.iManufacturer, (unsigned char*)usb_descriptor_buf, sizeof(usb_descriptor_buf));
                        usb_device_info.manufacturer_string.assign(usb_descriptor_buf);
                    }

                    if(usb_desc.iProduct)
                    {
                        libusb_get_string_descriptor_ascii(usb_handle, usb_desc.iProduct, (unsigned char*)usb_descriptor_buf, sizeof(usb_descriptor_buf));
                        usb_device_info.product_string.assign(usb_descriptor_buf);
                    }

                    libusb_close(usb_handle);
                }

                usb_info.push_back(usb_device_info);
            }
        }

        libusb_free_device_list(usb_devices, 1);
    }

    return(usb_info);
}

LogManager* ResourceManager::GetLogManager()
{
    return LogManager::get();
}

std::vector<RGBController*>& ResourceManager::GetRGBControllers()
{
    return(rgb_controllers);
}

std::vector<RGBControllerInterface*>& ResourceManager::GetRGBControllerInterfaces()
{
    return(rgb_controller_interfaces);
}

SettingsManager* ResourceManager::GetSettingsManager()
{
    return(settings_manager);
}

void ResourceManager::SetConfigurationDirectory(const filesystem::path &directory)
{
    config_dir = directory;
    settings_manager->LoadSettings(directory / "TurnOffRGB.json");
    LogManager::get()->Configure(settings_manager->GetSettings("LogManager"), GetConfigurationDirectory());
}

/*---------------------------------------------------------*\
| Callback Registration Functions                           |
\*---------------------------------------------------------*/
void ResourceManager::RegisterResourceManagerCallback(ResourceManagerCallback new_callback, void * new_callback_arg)
{
    ResourceManagerCallbackMutex.lock();

    for(size_t idx = 0; idx < ResourceManagerCallbacks.size(); idx++)
    {
        if(ResourceManagerCallbacks[idx] == new_callback && ResourceManagerCallbackArgs[idx] == new_callback_arg)
        {
            ResourceManagerCallbackMutex.unlock();

            LOG_TRACE("[%s] Tried to register an already registered ResourceManager callback, skipping.  Total callbacks registered: %d", RESOURCEMANAGER, ResourceManagerCallbacks.size());

            return;
        }
    }

    ResourceManagerCallbacks.push_back(new_callback);
    ResourceManagerCallbackArgs.push_back(new_callback_arg);

    ResourceManagerCallbackMutex.unlock();

    LOG_TRACE("[%s] Registered ResourceManager callback.  Total callbacks registered: %d", RESOURCEMANAGER, ResourceManagerCallbacks.size());
}

void ResourceManager::UnregisterResourceManagerCallback(ResourceManagerCallback callback, void * callback_arg)
{
    ResourceManagerCallbackMutex.lock();

    for(size_t idx = 0; idx < ResourceManagerCallbacks.size(); idx++)
    {
        if(ResourceManagerCallbacks[idx] == callback && ResourceManagerCallbackArgs[idx] == callback_arg)
        {
            ResourceManagerCallbacks.erase(ResourceManagerCallbacks.begin() + idx);
            ResourceManagerCallbackArgs.erase(ResourceManagerCallbackArgs.begin() + idx);
        }
    }

    ResourceManagerCallbackMutex.unlock();

    LOG_TRACE("[%s] Unregistered ResourceManager callback.  Total callbacks registered: %d", RESOURCEMANAGER, ResourceManagerCallbackArgs.size());
}

/*---------------------------------------------------------*\
| Functions to manage detection                             |
\*---------------------------------------------------------*/

unsigned int ResourceManager::GetDetectionPercent()
{
    return DetectionManager::get()->GetDetectionPercent();
}

std::string ResourceManager::GetDetectionString()
{
    return DetectionManager::get()->GetDetectionString();
}

void ResourceManager::RescanDevices()
{
    DetectionManager::get()->BeginDetection();
}

void ResourceManager::UpdateDeviceList()
{
    DeviceListChangeMutex.lock();

    /*-----------------------------------------------------*\
    | Clear the controller list                             |
    \*-----------------------------------------------------*/
    rgb_controllers.clear();
    rgb_controller_interfaces.clear();

    /*-----------------------------------------------------*\
    | Insert controllers into controller list.  Hold the    |
    | detection lock across the copy so a concurrent        |
    | registration cannot invalidate the list.               |
    \*-----------------------------------------------------*/
    DetectionManager::get()->LockRGBControllers();
    rgb_controllers             = DetectionManager::get()->GetRGBControllers();
    DetectionManager::get()->UnlockRGBControllers();

    /*-----------------------------------------------------*\
    | Synchronize interfaces with controllers               |
    \*-----------------------------------------------------*/
    rgb_controller_interfaces.reserve(rgb_controllers.size());

    for(RGBController* rgb_controller : rgb_controllers)
    {
        rgb_controller_interfaces.push_back((RGBControllerInterface*)rgb_controller);
    }

    /*-----------------------------------------------------*\
    | Signal list has changed                               |
    \*-----------------------------------------------------*/
    DeviceListChangeMutex.unlock();

    /*-----------------------------------------------------*\
    | Signal device list update                             |
    \*-----------------------------------------------------*/
    SignalResourceManagerUpdate(RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED);
}

void ResourceManager::WaitForDetection()
{
    DetectionManager::get()->WaitForDetection();
}

/*---------------------------------------------------------*\
| Function to signal update callbacks                       |
\*---------------------------------------------------------*/
void ResourceManager::SignalResourceManagerUpdate(unsigned int update_reason)
{
    ResourceManagerCallbackMutex.lock();

    for(std::size_t callback_idx = 0; callback_idx < ResourceManagerCallbacks.size(); callback_idx++)
    {
        ResourceManagerCallbacks[callback_idx](ResourceManagerCallbackArgs[callback_idx], update_reason);
    }

    ResourceManagerCallbackMutex.unlock();

    LOG_TRACE("[%s] ResourceManager update signalled: %d", RESOURCEMANAGER, update_reason);
}

void ResourceManager::SetupConfigurationDirectory()
{
    config_dir.clear();

    const wchar_t* appdata = _wgetenv(L"APPDATA");
    if(appdata != NULL)
    {
        config_dir = appdata;
    }

    /*-----------------------------------------------------*\
    | If a configuration directory was found, append         |
    | TurnOffRGB                                             |
    \*-----------------------------------------------------*/
    if(config_dir != "")
    {
        config_dir.append("TurnOffRGB");

        /*-------------------------------------------------*\
        | Create configuration directory if it doesn't exist |
        \*-------------------------------------------------*/
        filesystem::create_directories(config_dir);
    }
    else
    {
        config_dir = "./";
    }
}

void ResourceManager::Initialize()
{
    DetectionManager::get()->RegisterDetectionCallback(ResourceManagerDetectionCallback, this);
    DetectionManager::get()->BeginDetection();

    init_finished = true;
}

void ResourceManager::WaitForInitialization()
{
    /*-----------------------------------------------------*\
    | A reliable sychronization of this kind is impossible  |
    | without the use of a `barrier` implementation, which  |
    | is only introduced in C++20                           |
    \*-----------------------------------------------------*/
    while(!init_finished)
    {
        std::this_thread::sleep_for(1ms);
    };
}
