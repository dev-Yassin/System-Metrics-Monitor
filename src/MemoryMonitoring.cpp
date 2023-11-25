#include <ftxui/component/component.hpp>  // for components
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <string>  // for std::to_wstring
#include <utility>
#include <sys/sysinfo.h>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/mouse.hpp>
#include <iomanip>
#include <sys/statvfs.h>
#include <cmath>
#include "ftxui/screen/color.hpp"  // for Color
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include "../include/TitleComponent.hpp"
#include "../include/CpuMonitoring.hpp"
#include "../include/ProcessActivityMonitoring.hpp"
#include "../include/MemoryMonitoring.hpp"
#include "../include/DiskMonitoring.hpp"
#include "../include/Overview.hpp"
using namespace ftxui;

MemoryComponent::MemoryComponent(const std::wstring &title, int usage) : title_(title), usage(usage) {}

Element MemoryComponent::Render(){
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    this->terminal_width = (int)(size.ws_col);
    this->terminal_height = (int)(size.ws_row);
    std::string titleString(title_.begin(), title_.end());
    std::ofstream titleFile;
    titleFile.open ("title.txt");
    titleFile << titleString << std::endl;
    titleFile.close();

    auto my_border = borderStyled(ROUNDED, Color::RGB(238, 238, 238));
    auto text_color = color(Color::RGB(255, 211, 105));
    auto background_color = bgcolor(Color::RGB(34, 40, 49));
    auto space = std::wstring(1, L' ');
//        std::random_device rd;
//        std::mt19937 gen(rd());
    double memoryUsage;
    double rand = monitorMemory();
    if(usage == 1 or usage == 3) {
        // generate random memory usage
        memoryUsage = rand;
    } else if(usage == 2) {
        memoryUsage = 100.0 -rand;
    } else if (usage == 4) {
        memoryUsage = get_swap_usage();
    }
    int available_space = terminal_width - 33;
    if(terminal_width<= 38) {
        std::wstring memoryUsageString = std::to_wstring(memoryUsage) + L"%";
        Element memoryUsageElement = text((title_ + space) + L"[" + L"|" + L"] " + memoryUsageString) | hcenter | vcenter | flex;
        return memoryUsageElement | flex | text_color | my_border | background_color;
    }
    int numBars = static_cast<int>((memoryUsage * available_space) / 100);


    std::wstring bars(numBars, L'|');

    std::wstring spaces(available_space - numBars, L' ');
    std::wstringstream memoryUsageStream;
    memoryUsageStream << std::fixed << std::setprecision(2) << memoryUsage;
    std::wstring memoryUsageString = memoryUsageStream.str() + L"%";
    Element memoryUsageElement = text((title_ + space) + L"[" + bars + spaces + L"] " + memoryUsageString) | hcenter | vcenter | flex;
    return memoryUsageElement | flex | text_color | my_border | background_color;
}

void showMemoryUsage() {
    // create wstrings for all titles
    std::wstring main_title = L"System Metrics Monitor: Memory Usage";
    std::wstring free_memory_title = L"Free Memory:";
    std::wstring used_memory_title = L"Used Memory:";
    std::wstring instructions = L"F1 Overview F2 Cpu F3 Memory F4 Disk F5 ProcessInfo F6 Quit";
    auto screen = ScreenInteractive::Fullscreen();
    auto title_component = std::make_shared<TitleComponent>(main_title, L"");
//    auto memory_component = std::make_shared<MemoryComponent>(L"Overall Memory Usage:");
    auto FreeMemory_component = std::make_shared<MemoryComponent>(free_memory_title, 2);
    auto UsedMemory_component = std::make_shared<MemoryComponent>(used_memory_title, 3);
    auto swapMemory_component = std::make_shared<MemoryComponent>(L"Swap Memory", 4);
    auto instructions_component = std::make_shared<TitleComponent>(instructions, L"");

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
                                                        FreeMemory_component,
                                                        UsedMemory_component,
                                                        swapMemory_component
                                                }) | flex;

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
            showMemoryUsage();
        }
        else if(event == Event::F2) {
//             show Cpu Window:
            screen.ExitLoopClosure()();
            showCpuUsage();
        }
        else if (event == Event::F4) {
//             show Disk Window:
            screen.ExitLoopClosure()();
           showDiskUsage();
        }
        else if (event == Event::F5) {
            screen.ExitLoopClosure()();
            showProcessInfo(1, getProcesses());
        }
        else if(event == Event::F6) {
            // exit the program
            screen.ExitLoopClosure()();

        }
        return true;

    });

    screen.Loop(main_container);
}
double monitorMemory() {
    std::ifstream file("/proc/meminfo");
    if (!file) {
        std::cerr << "Failed to open /proc/meminfo" << std::endl;
        exit(-1);
    }

    std::string line;
    long int totalMemorySize = 0;
    long int freeMemorySize = 0;
    long int buffersSize = 0;
    long int cacheSize = 0;

    while (std::getline(file, line)) {
        if (line.compare(0, 9, "MemTotal:") == 0) {
            totalMemorySize = std::atoi(line.substr(10).c_str());
        } else if (line.compare(0, 8, "MemFree:") == 0) {
            freeMemorySize = std::atoi(line.substr(9).c_str());
        } else if (line.compare(0, 8, "Buffers:") == 0) {
            buffersSize = std::atoi(line.substr(9).c_str());
        } else if (line.compare(0, 7, "Cached:") == 0) {
            cacheSize = std::atoi(line.substr(8).c_str());
        }
    }

    file.close();



    long int usedMemorySize = totalMemorySize - freeMemorySize - buffersSize - cacheSize;

    // write to file for testing
    std::ofstream myfile;
    myfile.open ("MemoryInfo.txt");
    myfile << "used memory: " << usedMemorySize << " total memory: " << totalMemorySize<< "buffer size: " << buffersSize << "free memory: " << freeMemorySize << "buffers size: " << buffersSize << "cache size: " << cacheSize << std::endl;
    myfile.close();

    double usagePercentage = static_cast<double>(usedMemorySize) / totalMemorySize;
    return usagePercentage*100;
}


double get_swap_usage() {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        unsigned long long total_swap = info.totalswap / (1024 * 1024); // Convert to MB
        unsigned long long used_swap =  (info.totalswap - info.freeswap) / (1024 * 1024);
        return (double)used_swap / total_swap * 100;
    } else {
        std::cerr << "Failed to retrieve system information." << std::endl;
        exit(-1);
    }
}

