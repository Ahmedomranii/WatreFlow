cmake_minimum_required(VERSION 3.16)
project(WaterFlow VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(WaterFlow
    # --- Application (login, fenêtre principale, style) ---
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/logindialog.h
    src/style.h

    # --- Module Personnel ---
    modules/personnel/personnel.cpp modules/personnel/personnel.h modules/personnel/personnel.ui
    modules/personnel/barchart.cpp  modules/personnel/barchart.h

    # --- Module Livraisons ---
    modules/livraisons/livraisons.cpp modules/livraisons/livraisons.h modules/livraisons/livraisons.ui
)

target_include_directories(WaterFlow PRIVATE
    src
    modules/personnel
    modules/livraisons
)
target_link_libraries(WaterFlow PRIVATE Qt6::Widgets)
