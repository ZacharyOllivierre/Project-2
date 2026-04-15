cmake_minimum_required(VERSION 3.16)
project(Project2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

qt_add_executable(Project2
    main.cpp
    Database.cpp
    Database.h
    mainwindow.cpp
    mainwindow.h
    TeamInfoWidget.h
    TeamInfoWidget.cpp
)

target_link_libraries(Project2 PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
)
