//
// Created by M2 on 24/8/2023.
//

#ifndef SMON_TITLECOMPONENT_HPP
#define SMON_TITLECOMPONENT_HPP
#include "ftxui/component/component.hpp"
using namespace ftxui;

class TitleComponent : public ComponentBase {
private:
    std::wstring title_;
    std::wstring content_;

public:
    TitleComponent(const std::wstring &title, const std::wstring &content);

    Element Render() override;
};
#endif //SMON_TITLECOMPONENT_HPP
