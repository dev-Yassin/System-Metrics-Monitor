#include <ftxui/component/component.hpp>  // for components
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <ftxui/component/captured_mouse.hpp>
#include "../include/Overview.hpp"
#include "../include/TitleComponent.hpp"
#include "../include/CpuMonitoring.hpp"
#include "../include/MemoryMonitoring.hpp"
#include "../include/DiskMonitoring.hpp"
#include "../include/ProcessActivityMonitoring.hpp"
using namespace ftxui;

void showOverview() {
    std::wstring title = L"totalUsage";
    int use_switch = 1;
    double cpu_usage_default = 0.0;
    CpuMonitoring cpuMonitoringInstance(title, use_switch, cpu_usage_default);
    std::map<std::string, double> cpu_usage = cpuMonitoringInstance.get_cpu_usage();
    double total_usage = cpu_usage["total"];
    auto screen = ScreenInteractive::Fullscreen();
    auto title_component = std::make_shared<TitleComponent>(L"System Metrics Monitor: Overview", L"");
    auto cpu_component = std::make_shared<CpuMonitoring>(L"CPU Usage:", 1, total_usage);
    auto memory_component = std::make_shared<MemoryComponent>(L"Memory Usage:", 1);
    auto disk_component = std::make_shared<DiskComponent>(L"Disk Usage:", 1, 0.0, "");
    auto instructions_component = std::make_shared<TitleComponent>(L"F1 Overview F2 Cpu F3 Memory F4 Disk F5 ProcessInfo F6 Quit", L"");
    auto handleCustomEvent = [&screen]() {
        // Redraw the screen
        screen.PostEvent(Event::Custom);
    };

    // Start a timer to trigger the custom event after 2 seconds
    std::thread timerThread([&handleCustomEvent]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        handleCustomEvent();
    });
    // Create the main container and add the components.
    // Create a vertical container for the upper section of the screen.
    auto upper_container = Container::Vertical({
                                                       title_component
                                               });


    // Create a horizontal container for the CPU, Memory, and Disk components.
    auto middle_container = Container::Vertical({
                                                        cpu_component,
                                                        memory_component,
                                                        disk_component
                                                }) | flex;



    // Create a vertical container for the lower section of the screen.
    auto lower_container = Container::Vertical({
                                                       instructions_component
                                               });

    // Create a vertical container for the main screen, combining the upper, middle, and lower containers.
    // lower cona
    auto main_container = Container::Vertical({
                                                      upper_container,
                                                      middle_container,
                                                      lower_container
                                              });
    main_container |= CatchEvent([&](Event event) {
        if(event == Event::F2) {
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
            showCpuUsage();
        }

        else if(event == Event::Custom) {
            screen.ExitLoopClosure()();
            CpuUseCounter =0;
            showOverview();
        }
        else if(event == Event::F3) {
            screen.ExitLoopClosure()();
            CpuUseCounter = 0;
            showMemoryUsage();
        }
        else if(event == Event::F4) {
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
            timerThread.join();
            screen.ExitLoopClosure()();
            system("reset");
            exit(0);

        }

        return true;
    });


    screen.Loop(main_container);
}


