

#ifndef SMON_PROCESSACTIVITYMONITORING_HPP
#define SMON_PROCESSACTIVITYMONITORING_HPP
#include <ftxui/component/component.hpp>  // for ComponentBase
using namespace ftxui;
struct Process {
    int pid;
    int ppid;
    std::string user;
    std::string command;
    double cpu_usage;
    double memory_usage;
    double read_speed;
    double write_speed;
};

void showProcessInfo(int start, std::vector<Process> processes);

std::vector<Process> getProcesses();

class ProcessActivityMonitoring : public ComponentBase {
private:
    Process process;
    int usage;

public:
    explicit ProcessActivityMonitoring(Process process, int usage);

    std::wstring swipeRight(const std::wstring &original, int size);

    Element Render() override;
};

#endif //SMON_PROCESSACTIVITYMONITORING_HPP
