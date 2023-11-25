#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include <ftxui/dom/elements.hpp>  // for elements
#include <string>  // for std::to_wstring
#include "../include/TitleComponent.hpp"
// #pragma once
using namespace ftxui;



TitleComponent::TitleComponent(const std::wstring &title, const std::wstring &content) : title_(title), content_(content) {}

    // make a function that gets the terminal size


Element TitleComponent::Render(){
    auto my_border = borderStyled(ROUNDED, Color::RGB(238, 238, 238));
    auto text_color = color(Color::RGB(255, 211, 105));
    auto background_color = bgcolor(Color::RGB(34, 40, 49));
    auto space = std::wstring(1, L' ');
    return vbox({
                        text((title_ + space) + content_) | hcenter | vcenter | flex,
                }) |
           flex | text_color | my_border | background_color;
}