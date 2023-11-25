//
// Created by M2 on 24/8/2023.
//

#ifndef SMON_MEMORYMONITORING_HPP
#define SMON_MEMORYMONITORING_HPP

#include <ftxui/component/component.hpp>  // for ComponentBase


using namespace ftxui;
double get_swap_usage();
void showMemoryUsage();
double monitorMemory();
class MemoryComponent : public ComponentBase {
private:

    int terminal_width;
    int terminal_height;
    int usage;
    std::wstring title_;
public:
    MemoryComponent(const std::wstring &title, int usage);
    Element Render() override;

    };


#endif //SMON_MEMORYMONITORING_HPP
