//
// Created by M2 on 24/8/2023.
//

#ifndef SMON_CPUMONITORING_HPP
#define SMON_CPUMONITORING_HPP
#include <ftxui/component/component.hpp>  // for ComponentBase
#include <ftxui/component/event.hpp>  // for Event
#include <string>  // for std::wstring
#include <utility>  // for std::pair
#include <map>
using namespace ftxui;


extern int CpuUseCounter;
class CpuMonitoring : public ftxui::ComponentBase {
private:
    std::wstring title_;
    int use_switch;
    int terminal_width;
    int terminal_height;
    double cpuUsage;

public:
    CpuMonitoring(const std::wstring &title, int use_switch, double cpuUsage);
    Element Render() override;
    static std::vector<long> get_cpu_times();
    std::vector<long> get_idle_times();
    std::map<std::string, double> get_cpu_usage();
//    double get_cpu_usage();
};

void showCpuUsage();
#endif //SMON_CPUMONITORING_HPP
