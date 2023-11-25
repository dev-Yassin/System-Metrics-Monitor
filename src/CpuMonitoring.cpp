#include <ftxui/component/component.hpp>  // for components
#include <ftxui/component/event.hpp>  // for Event
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <string>  // for std::to_wstring
#include <utility>
#include <ftxui/component/captured_mouse.hpp>
#include <iomanip>
#include "ftxui/screen/color.hpp"  // for Color
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <random>
#include <map>
#include "../include/CpuMonitoring.hpp"
#include "../include/TitleComponent.hpp"
#include "../include/DiskMonitoring.hpp"
#include "../include/ProcessActivityMonitoring.hpp"
#include "../include/Overview.hpp"
#include "../include/MemoryMonitoring.hpp"
//#pragma once
using namespace ftxui;


int CpuUseCounter = 0;
CpuMonitoring:: CpuMonitoring(const std::wstring &title, int use_switch, double cpuUsage) : title_(title), use_switch(use_switch) , cpuUsage(cpuUsage) {}
Element CpuMonitoring::Render() {
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    this->terminal_width = (int)(size.ws_col);
    this->terminal_height = (int)(size.ws_row);
//    double cpuUsage;



    auto my_border = borderStyled(ROUNDED, Color::RGB(238, 238, 238));
    auto text_color = color(Color::RGB(255, 211, 105));
    auto background_color = bgcolor(Color::RGB(34, 40, 49));
    auto space = std::wstring(1, L' ');
//        double cpuUsage = this ->cpu_usage;
    std::random_device rd;
    std::mt19937 gen(rd());

    if(use_switch==2) {
        terminal_width/=2;
        // cpu_usage is a random number between 20 and 50 use rd to generate a random number
//        cpuUsage = std::uniform_real_distribution<>(0, 5.0)(gen);
    } 
//    
//    else {
//
//        if(CpuUseCounter == 0) {
//            cpuUsage = 0
//        } else {
//            cpuUsage = get_cpu_usage();
//        }
//    }

    int available_space = terminal_width - 30;

    if(terminal_width<= 35) {
        std::wstringstream cpuUsageStream;
        cpuUsageStream << std::fixed << std::setprecision(2) << cpuUsage;
        std::wstring cpuUsageString = cpuUsageStream.str() + L"%";
        Element cpuUsageElement = text((title_ + space) + L"[" + L"|" + L"] " + cpuUsageString) | hcenter | vcenter | flex;
        return cpuUsageElement | flex | text_color | my_border | background_color;
    }
    int numBars = static_cast<int>((cpuUsage * available_space) / 100);


    std::wstring bars(numBars, L'|');

    std::wstring spaces(available_space - numBars, L' ');
    std::wstringstream cpuUsageStream;
    cpuUsageStream << std::fixed << std::setprecision(2) << cpuUsage;
    std::wstring cpuUsageString = cpuUsageStream.str() + L"%";
    Element cpuUsageElement = text((title_ + space) + L"[" + bars + spaces + L"] " + cpuUsageString) | hcenter | vcenter | flex;


    return cpuUsageElement | flex | text_color | my_border | background_color;
}
//std::pair<double, double> CpuMonitoring::get_cpu_times() {
//    std::ifstream file("/proc/stat");
//    std::string cpu;
//    double user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
//    file >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
//    double total = user + nice + system + idle + iowait + irq + softirq + steal;
//    return std::make_pair(total, idle);
//}
//double CpuMonitoring::get_cpu_usage() {
//    if(CpuUseCounter==0) {
//        CpuUseCounter++;
//        return 0.0;
//    }
//    auto cpu_times_start = get_cpu_times();
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    auto cpu_times_end = get_cpu_times();
//    double total = cpu_times_end.first - cpu_times_start.first;
//    double idle = cpu_times_end.second - cpu_times_start.second;
//    return 100.0 * (1.0 - idle / total);
//}
std::vector<long> CpuMonitoring::get_cpu_times() {
    std::ifstream proc_stat("/proc/stat");
    std::string cpu_name;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    std::vector<long> cpu_times;

    while (proc_stat >> cpu_name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice) {
        if (cpu_name.substr(0,3) == "cpu") {
            cpu_times.push_back(user + nice + system + idle + iowait + irq + softirq + steal);
        }
    }

    return cpu_times;
}

std::vector<long> CpuMonitoring::get_idle_times() {
    std::ifstream proc_stat("/proc/stat");
    std::string cpu_name;
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    std::vector<long> idle_times;

    while (proc_stat >> cpu_name >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice) {
        if (cpu_name.substr(0,3) == "cpu") {
            idle_times.push_back(idle + iowait);
        }
    }

    return idle_times;
}

std::map<std::string, double> CpuMonitoring::get_cpu_usage() {
    std::vector<long> total_time_start = CpuMonitoring::get_cpu_times();
    std::vector<long> idle_time_start = CpuMonitoring::get_idle_times();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<long> total_time_end = CpuMonitoring::get_cpu_times();
    std::vector<long> idle_time_end = CpuMonitoring::get_idle_times();

    std::map<std::string, double> cpu_usages;
    for (size_t i = 0; i < total_time_start.size(); ++i) {
        long totald = total_time_end[i] - total_time_start[i];
        long idled = idle_time_end[i] - idle_time_start[i];

        std::string cpu_name = i == 0 ? "total" : "CPU" + std::to_string(i - 1);
        cpu_usages[cpu_name] = 100.0 * (totald - idled) / totald;
    }

    return cpu_usages;
}

void showCpuUsage() {
    // create wstrings for all titles
//    if(CpuUseCounter == 0) {
//        CpuUseCounter++;
//    } else {
//
//        if(
//    }
    std::wstring title = L"My Cpu Monitoring";
    int use_switch = 1; // replace with actual value
    double cpu_usage_default = 0.0; // replace with actual value
    CpuMonitoring cpuMonitoringInstance(title, use_switch, cpu_usage_default);
    std::map<std::string, double> cpu_usage = cpuMonitoringInstance.get_cpu_usage();
    double total_usage = cpu_usage["total"];
    std::ofstream outputFile("cpu_log.txt");
    outputFile << total_usage;
    outputFile.close();
    std::wstring main_title = L"System Metrics Monitor: Cpu Usage";
    std::wstring instructions = L"F1 Overview F2 Cpu F3 Memory F4 Disk F5 ProcessInfo F6 Quit";
    auto screen = ScreenInteractive::Fullscreen();
    auto title_component = std::make_shared<TitleComponent>(main_title, L"");
    auto cpu_component = std::make_shared<CpuMonitoring>(L"Total Usage: ", 1, total_usage);
    auto instructions_component = std::make_shared<TitleComponent>(instructions, L"");
    auto insufficient_height_container = std::make_shared<TitleComponent>(L"Terminal window is too small to display all cores", L"");
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    int terminal_height = (int)(size.ws_row);
    int number_of_cores = cpu_usage.size()-1;
    int needed_height = ((number_of_cores/2) * 3) + 9;
    auto handleCustomEvent = [&screen]() {
        // Redraw the screen
        screen.PostEvent(Event::Custom);
      };

      // Start a timer to trigger the custom event after 2 seconds
      std::thread timerThread([&handleCustomEvent]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        handleCustomEvent();
      });
    // make a vector of cpu components
    std::vector<std::shared_ptr<CpuMonitoring>> cpu_components;
    for (int i = 0; i < number_of_cores; i++) {
        std::wstring title = L"Core " + std::to_wstring(i+1) + L":";
        auto core_component = std::make_shared<CpuMonitoring>(title, 2, cpu_usage[std::string("CPU") + std::to_string(i)]);
        
        cpu_components.push_back(core_component);
    }

    // handle the display of the cpu components in case we do not know how many cores the system has
    auto mid_upper_container = Container::Vertical({
                                                           cpu_component
                                                   });
    auto middle_container = Container::Vertical({});
    if(number_of_cores %2 == 0) {
        for(int i = 0; i < number_of_cores; i++) {
            middle_container-> Add(Container::Horizontal({
                                                                 cpu_components[i], cpu_components[i+1] }) | flex);
            i++;
        }
    } else {
        for(int i = 0; i < number_of_cores-1; i++) {
            middle_container-> Add(Container::Horizontal({
                                                                 cpu_components[i], cpu_components[i+1] }) | flex);
            i++;
        }
        middle_container-> Add(Container::Horizontal({
                                                             cpu_components[number_of_cores-1] }) | flex);
    }
    middle_container |= flex;


    auto upper_container = Container::Vertical({
                                                       title_component
                                               });
    auto lower_container = Container::Vertical({
                                                       instructions_component
                                               });



    auto main_container = Container::Vertical({
                                                      upper_container,
                                                      mid_upper_container,
                                                      middle_container,
                                                      lower_container
                                              });
    if(needed_height > terminal_height) {
        main_container = Container::Vertical({
                                                     upper_container,
                                                     mid_upper_container,
                                                     insufficient_height_container,
                                                     lower_container
                                             });
    }

    main_container |= CatchEvent([&](Event event) {
        if(event == Event::F1) {
            // show Overview Window:
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
           showOverview();
        }
        if(event == Event::Custom) {
            screen.ExitLoopClosure()();
            CpuUseCounter =0;
            showCpuUsage();
        }
        else if(event == Event::F3) {
            // show Memory Window:
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
           showMemoryUsage();
        }
        else if (event == Event::F4) {
//             show Disk Window:
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
           showDiskUsage();
        }
        else if (event == Event::F5) {
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
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