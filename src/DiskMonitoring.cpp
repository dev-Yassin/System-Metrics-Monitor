#include <ftxui/component/component.hpp>  // for components
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <string>  // for std::to_wstring
#include <ftxui/component/captured_mouse.hpp>
#include <iomanip>
#include <sys/statvfs.h>
#include <cmath>
#include "ftxui/screen/color.hpp"  // for Color
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <random>

//#pragma once
#include "../include/TitleComponent.hpp"
#include "../include/DiskMonitoring.hpp"
#include "../include/CpuMonitoring.hpp"
#include "../include/MemoryMonitoring.hpp"
#include "../include/Overview.hpp"
#include "../include/ProcessActivityMonitoring.hpp"
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <dirent.h>
#include <cstring>
#include <limits>
#include <utility>
#include <sstream>
using namespace ftxui;



DiskSpaceUsage calculateDiskSpaceUsage() {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        std::cerr << "Error getting filesystem statistics." << std::endl;
        exit(1); // Exit with an error code
    }

    unsigned long blockSize = stat.f_frsize;
    unsigned long totalBlocks = stat.f_blocks;
    unsigned long freeBlocks = stat.f_bfree;
    unsigned long usedBlocks = totalBlocks - freeBlocks;
    unsigned long totalSize = totalBlocks * blockSize;
    unsigned long usedSize = usedBlocks * blockSize;

    const char* units[] = {"bytes", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(totalSize);

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        ++unitIndex;
    }

    // Fill the DiskSpaceUsage struct
    DiskSpaceUsage usage;
    usage.percentageUsed = static_cast<double>(usedSize) / static_cast<double>(totalSize) * 100.0;
    usage.sizes = {static_cast<double>(totalSize) / pow(1024.0, unitIndex),
                   static_cast<double>(usedSize) / pow(1024.0, unitIndex)};
    usage.unit = units[unitIndex];

    return usage;
}



//DiskComponent::DiskComponent(const std::wstring &title, int usage, double fullMemory, std::string diskName) : title_(title), usage(usage), fullMemory(fullMemory){}

DiskComponent::DiskComponent(const wchar_t *title, int usage, double fullMemory, std::string diskName)
        : title_(std::move(title)), usage(usage), fullMemory(fullMemory), diskName(std::move(diskName)) {}


std::map<std::wstring, std::pair<double, double>> getDiskInfo() {
    std::map<std::wstring, std::pair<double, double>> diskInfo;

    DIR* dirp = opendir("/sys/block");
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        std::wstring diskName = std::wstring(dp->d_name, dp->d_name + strlen(dp->d_name));

        std::ifstream sizeFile(std::string("/sys/block/") + dp->d_name + "/size");
        long long totalSize;
        sizeFile >> totalSize;

        std::ifstream statFile(std::string("/sys/block/") + dp->d_name + "/stat");
        long long readSectors, writeSectors;
        statFile.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // Skip the first field
        statFile >> readSectors;
        statFile.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // Skip the next 3 fields
        statFile >> writeSectors;

        long long usedSize = readSectors + writeSectors;
        long long freeSize = totalSize - usedSize;

        // Convert sectors to bytes
        double totalSizeBytes = totalSize * 512.0;
        double freeSizeBytes = freeSize * 512.0;

        diskInfo[diskName] = std::make_pair(freeSizeBytes, totalSizeBytes);
    }
    closedir(dirp);

    return diskInfo;
}
Element DiskComponent::Render() {
    auto my_border = borderStyled(ROUNDED, Color::RGB(238, 238, 238));
    auto text_color = color(Color::RGB(255, 211, 105));
    auto background_color = bgcolor(Color::RGB(34, 40, 49));
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    this->terminal_width = (int) (size.ws_col);
    this->terminal_height = (int) (size.ws_row);
    if (usage == 2) {
        // we will return an Element that containing the string: "Current Devices:
        std::wstring currentDevices = L"Current Devices:";
        Element currentDevicesElement = text(currentDevices) | vcenter | hcenter;
        return currentDevicesElement | flex | text_color | my_border | background_color;
    } else if (usage == 3) {
        fullMemory /= (1024.0 * 1024.0 * 1024.0);
        std::stringstream fullMemoryStream;
            fullMemoryStream << std::fixed << std::setprecision(2) << fullMemory;
        auto deviceInfo = diskName + ":   " +/* std::to_string(usedMemory) + " / " +*/ fullMemoryStream.str() + " GB" + "\n";
        Element diskDevicesElement = text(deviceInfo) | vcenter | hcenter;
        return diskDevicesElement | flex | text_color | my_border | background_color;
    }else if(usage == 4) {
        std::string space = "    ";
        auto deviceInfo = "disk" + space +/* std::to_string(usedMemory) + " / " +*/ "size";
        Element diskDevicesElement = text(deviceInfo) | vcenter | hcenter;;
        return diskDevicesElement | flex | text_color | my_border | background_color;

    }else {

        DiskSpaceUsage diskSpaceUsage = calculateDiskSpaceUsage();


        auto space = std::wstring(1, L' ');
        std::random_device rd;
        std::mt19937 gen(rd());
        // double diskUsage = std::uniform_real_distribution<>(25.0, 26.0)(gen);
        double diskUsage = diskSpaceUsage.percentageUsed;
        int available_space = terminal_width - 33;
        if (terminal_width <= 38) {
            std::wstringstream diskUsageStream;
            diskUsageStream << std::fixed << std::setprecision(2) << diskUsage;
            std::wstring diskUsageString = diskUsageStream.str() + L"%";
            Element diskUsageElement =
                    text((title_ + space) + L"[" + L"|" + L"] " + diskUsageString) | hcenter | vcenter | flex;
            return diskUsageElement | flex | text_color | my_border | background_color;
        }
        int numBars = static_cast<int>((diskUsage * available_space) / 100);


        std::wstring bars(numBars, L'|');

        std::wstring spaces(available_space - numBars, L' ');
        std::wstringstream diskUsageStream;
        diskUsageStream << std::fixed << std::setprecision(2) << diskUsage;
        std::wstring diskUsageString = diskUsageStream.str() + L"%";
        Element diskUsageElement =
                text((title_ + space) + L"[" + bars + spaces + L"] " + diskUsageString) | hcenter | vcenter | flex;
        return diskUsageElement | flex | text_color | my_border | background_color;
    }
}
auto make_device_components() {
    auto disk_device_table_title = std::make_shared<DiskComponent>(L"", 4, 0.0, "");
    std::map<std::wstring, std::pair<double, double>> devices = getDiskInfo();
    auto device_container = Container::Vertical({});
    device_container -> Add(disk_device_table_title | flex);
    std::string deviceInfo;
//    int number_of_devices = devices.size();
//    for(int i = 0; i < number_of_devices; i++);

    for (const auto &entry: devices) {
        std::string diskName = to_string(entry.first);
//        double freeMemory = entry.second.first;
        double fullMemory = entry.second.second;
        auto disk_device = std::make_shared<DiskComponent>(L"", 3, fullMemory, diskName);

        device_container ->Add(disk_device | flex);
    }
    return device_container;
}

void showDiskUsage() {
    // create wstrings for all titles
    std::wstring main_title = L"System Metrics Monitor: Disk Usage";
    std::wstring instructions = L"F1 Overview F2 Cpu F3 Memory F4 Disk F5 ProcessInfo F6 Quit";
    auto screen = ScreenInteractive::Fullscreen();
    auto title_component = std::make_shared<TitleComponent>(main_title, L"");
    auto disk_component = std::make_shared<DiskComponent>(L"Total Usage: ", 1, 0.0, "");
    auto instructions_component = std::make_shared<TitleComponent>(instructions, L"");
    auto disk_devices_title = std::make_shared<DiskComponent>(L"Disk Devices: ", 2, 0.0, "");
//    auto disk_devices = std::make_shared<DiskComponent>(L"", 3, 0.0, "");
    auto device_components = make_device_components();

    auto handleCustomEvent = [&screen]() {
        // Redraw the screen
        screen.PostEvent(Event::Custom);
    };

    // Start a timer to trigger the custom event after 2 seconds
    std::thread timerThread([&handleCustomEvent]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        handleCustomEvent();
    });
    auto upper_container = Container::Vertical({
                                                       title_component
                                               });
    auto middle_container = Container::Vertical({
                                                        disk_component,
                                                        Container::Horizontal({
                                                                                      Container::Vertical({
                                                                                                                  disk_devices_title
                                                                                                          }) | flex,
                                                                                      Container::Vertical({
                                                                                                                  device_components
                                                                                                          }) | flex,

                                                                              }) | flex
                                                });
    auto lower_container = Container::Vertical({
                                                       instructions_component
                                               });

    auto main_container = Container::Vertical({
                                                      upper_container,
                                                      middle_container,
                                                      lower_container
                                              });

        main_container |= CatchEvent([&](Event event) {
            if(event == Event::F1) {
                // show Overview Window:
                screen.ExitLoopClosure()();
               showOverview();
            }
            else if(event == Event::Custom) {
                screen.ExitLoopClosure()();
                CpuUseCounter =0;
                showDiskUsage();
            }
            else if(event == Event::F2) {
                // show Cpu Window:
                screen.ExitLoopClosure()();
               showCpuUsage();
            }
            else if(event == Event::F3) {
                // show Memory Window:
                screen.ExitLoopClosure()();
               showMemoryUsage();
            }
            else if (event == Event::F5) {
                screen.ExitLoopClosure()();
               showProcessInfo(1, getProcesses());

            }
            else if (event == Event::F6) {
                // exit without leaving the terminal in a weird state.
                screen.ExitLoopClosure()();

            }
            return true;
        });
    screen.Loop(main_container);
}


