cmake_minimum_required(VERSION 3.25)
project(smon)

# ------------------------------------------------------------------------------
# Add the following lines:
find_package(ftxui REQUIRED)
include_directories(${FTXUI_INCLUDE_DIRS})
# Include the root directory of the project
include_directories(${CMAKE_SOURCE_DIR})
add_executable(smon 
    main.cpp 
    src/CpuMonitoring.cpp
    src/MemoryMonitoring.cpp
    src/DiskMonitoring.cpp
    src/Overview.cpp
    src/ProcessActivityMonitoring.cpp
    src/TitleComponent.cpp)

target_link_libraries(smon PRIVATE ftxui::screen ftxui::dom ftxui::component)
set_target_properties(smon PROPERTIES
    CXX_STANDARD_REQUIRED ON
)
