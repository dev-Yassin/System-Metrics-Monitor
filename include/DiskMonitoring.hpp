//
// Created by M2 on 24/8/2023.
//

#ifndef SMON_DISKMONITORING_HPP
#define SMON_DISKMONITORING_HPP
#include <ftxui/component/component.hpp>  // for ComponentBase

using namespace ftxui;
struct DiskSpaceUsage {
    double percentageUsed;
    std::vector<double> sizes;
    std::string unit;

};
DiskSpaceUsage calculateDiskSpaceUsage();

class DiskComponent : public ComponentBase {
private:
    std::wstring title_;
    int terminal_width;
    int terminal_height;
    int usage;
    double fullMemory;
    std::string diskName;

public:
//    explicit DiskComponent(const std::wstring &title, int usage, double fullMemory, std::string diskName);
    explicit DiskComponent(const wchar_t *title, int usage, double fullMemory, std::string diskName);

    struct DiskDevice {
        std::wstring name;
        double percentageUsed;
        double totalSize;
        double usedSize;
        double freeSize;
        std::wstring unit;
        int index;
    };
    static std::vector<DiskDevice> get_devices_data();
    Element Render() override;
};
void showDiskUsage();




#endif //SMON_DISKMONITORING_HPP
