#include <ftxui/component/component.hpp>  // for components
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <string>  // for std::to_wstring
#include <ftxui/component/captured_mouse.hpp>
#include <iomanip>
#include <cmath>
#include "ftxui/screen/color.hpp"  // for Color
#include <sys/ioctl.h>
#include <unistd.h>
#include <sstream>
#include "../include/TitleComponent.hpp"
#include "../include/ProcessActivityMonitoring.hpp"
#include "../include/Overview.hpp"
#include "../include/CpuMonitoring.hpp"
#include "../include/DiskMonitoring.hpp"
#include "../include/MemoryMonitoring.hpp"





ProcessActivityMonitoring::ProcessActivityMonitoring(Process process, int usage) : process(process), usage(usage) {}

std::wstring ProcessActivityMonitoring::swipeRight(const std::wstring &original, int size) {
    if (size <= original.size()) {
        return original;
    }

    std::wstring result(size, L' ');  // Create a wstring of specified size filled with spaces

    // Copy the characters from the original string to the right side of the result string
    for (int i = 0; i < original.size(); ++i) {
        result[size - original.size() + i] = original[i];
    }

    return result;
}

Element ProcessActivityMonitoring::Render() {

    auto my_border = borderStyled(ROUNDED, Color::RGB(238, 238, 238));
    auto text_color = color(Color::RGB(255, 211, 105));
    auto background_color = bgcolor(Color::RGB(34, 40, 49));
    auto space = std::wstring(2, L' ');
    // the pid for example should be right aligned
    if (usage == 2) {
        // return the titles
        std::wstring pid = L"PID";
        pid = swipeRight(pid, 5);
        std::wstring ppid = L"PPID";
        ppid = swipeRight(ppid, 5);
        std::wstring cpu_usage = L"CPU%";
        cpu_usage = swipeRight(cpu_usage, 5);
        std::wstring memory_usage = L"MEM%";
        memory_usage = swipeRight(memory_usage, 5);
        std::wstring read_speed = L"READ";
        read_speed = swipeRight(read_speed, 5);
        std::wstring write_speed = L"WRITE";
        write_speed = swipeRight(write_speed, 5);
        std::wstring command = L"COMMAND";
        // user field was removed for later addition
        Element processElement =
                text(pid + space + ppid + space + cpu_usage + space + memory_usage /*+ space + user */+ space + command) | vcenter;
        return processElement | flex | text_color | my_border | background_color;
    }
    std::wstring pid = std::to_wstring(process.pid);
    pid = swipeRight(pid, 5);
    std::wstring ppid = std::to_wstring(process.ppid);
    ppid = swipeRight(ppid, 5);
    // std::wstring user = std::wstring(process.user.begin(), process.user.end());
    // user = swipeRight(user, 8);
    std::wstring cpu_usage = std::to_wstring(process.cpu_usage);
    std::wstring memory_usage = std::to_wstring(process.memory_usage);
    std::wstringstream cpuUsageStream;
    cpuUsageStream << std::fixed << std::setprecision(2) << process.cpu_usage;
    cpu_usage = cpuUsageStream.str();
    cpu_usage = swipeRight(cpu_usage, 5);
    std::wstringstream memoryUsageStream;
    memoryUsageStream << std::fixed << std::setprecision(2) << process.memory_usage;
    std::wstring read_speed = std::to_wstring(process.read_speed);
    std::wstring write_speed = std::to_wstring(process.write_speed);
    std::wstringstream readSpeedStream;
    memory_usage = memoryUsageStream.str();
    memory_usage = swipeRight(memory_usage, 5);
    std::wstring command = std::wstring(process.command.begin(), process.command.end());
    // user field was removed for later addition
    Element processElement =
            text(pid + space + ppid + space + cpu_usage + space + memory_usage /*+ space + user*/ + space + command) | vcenter | flex;
    return processElement | flex | text_color | my_border | background_color;
}
std::vector<Process> getProcesses() {
    std::vector<Process> processes;
    // feed random data to the vector
    for (int i = 0; i < 20; i++) {
        Process p;
        p.user = "user";
        p.pid = 12 * i ^ 2;
        p.ppid = i;
        p.cpu_usage = i;
        p.memory_usage = i;
        p.read_speed = 30.0 / ((double) i + 1);
        p.write_speed = 20.0 / (i + 1);
        p.command = "/Applications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/CApplications/Spotify.app/Contents/MacOS/Spotify/Contents/MacOS/Spotify/C";
        processes.push_back(p);
    }
    return processes;
}

void showProcessInfo(int start, std::vector<Process> processes) {
// create wstrings for all titles
    Process title;
    std::wstring main_title = L"System Metrics Monitor: Process Usage Information";
    std::wstring instructions = L"F1 Overview F2 Cpu F3 Memory F4 Disk F5 ProcessInfo F6 Quit";
    auto screen = ScreenInteractive::Fullscreen();
    auto title_component = std::make_shared<TitleComponent>(main_title, L"");
    auto processTableTitle = std::make_shared<ProcessActivityMonitoring>(title, 2);
    auto instructions_component = std::make_shared<TitleComponent>(instructions, L"");
    // create wstrings for all titles

    auto process_command = std::make_shared<TitleComponent>(L"F9 Sort", L"");
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    int terminal_height = (int) (size.ws_row);
    terminal_height -= 6;
    int max_to_show = terminal_height / 3;
    int left_to_show = processes.size() - start;
    // create new vector for the process components
    std::vector<std::shared_ptr<ProcessActivityMonitoring>> process_components;
    // create a process component for each process
    for (int i = start - 1; i < start + max_to_show - 1 && i < processes.size(); i++) {
        auto process_component = std::make_shared<ProcessActivityMonitoring>(processes[i], 1);
        process_components.push_back(process_component);
    }
    // create a container for the process components
    auto process_container = Container::Vertical({processTableTitle});
    // add all process components to the container
    for (int i = 1; i < process_components.size(); i++) {
        process_container->Add(process_components[i]);
    }
    auto handleCustomEvent = [&screen]() {
        // Redraw the screen
        screen.PostEvent(Event::Custom);
    };

    // Start a timer to trigger the custom event after 2 seconds
    std::thread timerThread([&handleCustomEvent]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        handleCustomEvent();
    });


    auto upper_container = Container::Horizontal({title_component, process_command});
//    auto middle_container = Container::Vertical({
//                                                        process_component
//                                                }) | flex;
    auto lower_container = Container::Vertical({instructions_component});

    auto main_container = Container::Vertical({upper_container, process_container | flex, lower_container});
    main_container |= CatchEvent([&](Event event) {
        if (event == Event::ArrowDown) {
//

            if (start <= processes.size() - 1 && left_to_show > max_to_show) {
                screen.ExitLoopClosure()();
//                screen.PostEvent(Event::Custom);
                showProcessInfo(start + 1, processes);
            }
        } else if (event == Event::ArrowUp) {
            if (start > 1) {
                screen.ExitLoopClosure()();
//                screen.PostEvent(Event::Custom);
                showProcessInfo(start - 1, processes);
            }
        } else if (event == Event::F1) {
            // show Overview Window:
            screen.ExitLoopClosure()();
            showOverview();
        }
        else if(event == Event::Custom) {
            screen.ExitLoopClosure()();
            CpuUseCounter =0;
            showProcessInfo(start, processes);
        }
        else if (event == Event::F2) {
            // show Cpu Window:
            screen.ExitLoopClosure()();
            showCpuUsage();
        } else if (event == Event::F3) {
            // show Memory Window:
            screen.ExitLoopClosure()();
            showMemoryUsage();
        } else if (event == Event::F4) {
            // show Disk Window:
            screen.ExitLoopClosure()();
            showDiskUsage();
        } else if (event == Event::F6) {
            // exit without leaving the terminal in a weird state.
            screen.ExitLoopClosure()();
            system("reset");
            std::cout << "\x1B[2J\x1B[H";
            exit(0);
            // call the system to clear the terminal


        } else if (event == Event::F9
                ) {
            // sort processes by cpu usage
            // remove all components from the process container
            std::sort(processes.begin(), processes.end(), [](Process a, Process b) {
                return a.pid >= b.pid;
            });
            screen.ExitLoopClosure()();
            showProcessInfo(start, processes);
        }
        return true;
    });
    screen.Loop(main_container);

}